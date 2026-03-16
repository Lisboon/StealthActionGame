// Copyright 2026, Lisboon. All Rights Reserved.

#include "AI/SAEnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interfaces/ISADetectable.h"

#if ENABLE_DRAW_DEBUG
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#endif

ASAEnemyAIController::ASAEnemyAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius                              = SightRadius;
	SightConfig->LoseSightRadius                          = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees             = PeripheralVisionAngle;
	SightConfig->SetMaxAge(5.f);
	SightConfig->AutoSuccessRangeFromLastSeenLocation     = 500.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	AIPerceptionComponent->ConfigureSense(*SightConfig);

	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange                              = HearingRange;
	HearingConfig->SetMaxAge(3.f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies    = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	AIPerceptionComponent->ConfigureSense(*HearingConfig);

	AIPerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
}

void ASAEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ASAEnemyAIController::OnPerceptionUpdated);
	}
}

void ASAEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

void ASAEnemyAIController::SetAlertState(ESAAlertState NewState)
{
	if (CurrentAlertState == NewState) return;

	CurrentAlertState = NewState;

	GetWorldTimerManager().ClearTimer(SuspiciousTimerHandle);
	GetWorldTimerManager().ClearTimer(SearchTimerHandle);

	switch (NewState)
	{
	case ESAAlertState::Suspicious:
		GetWorldTimerManager().SetTimer(SuspiciousTimerHandle, this,
			&ASAEnemyAIController::OnSuspiciousTimeout, SuspiciousTimeout, false);
		break;

	case ESAAlertState::Searching:
		GetWorldTimerManager().SetTimer(SearchTimerHandle, this,
			&ASAEnemyAIController::OnSearchTimeout, AlertSearchDuration, false);
		break;

	default: break;
	}

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(SABlackboardKeys::AlertState, static_cast<uint8>(NewState));

		if (NewState == ESAAlertState::Neutral)
		{
			BB->ClearValue(SABlackboardKeys::ThreatLocation);
			BB->ClearValue(SABlackboardKeys::ThreatActor);
		}
	}

#if ENABLE_DRAW_DEBUG
	if (GEngine)
	{
		const FString AlertStateName = UEnum::GetValueAsString(NewState);
		const FColor StateColor = [NewState]() -> FColor
		{
			switch (NewState)
			{
			case ESAAlertState::Neutral:    return FColor::Green;
			case ESAAlertState::Suspicious: return FColor::Yellow;
			case ESAAlertState::Alert:      return FColor::Red;
			case ESAAlertState::Searching:  return FColor::Orange;
			default:                        return FColor::White;
			}
		}();

		if (const APawn* ControlledPawn = GetPawn())
		{
			DrawDebugString(GetWorld(),
				ControlledPawn->GetActorLocation() + FVector(0.f, 0.f, 120.f),
				FString::Printf(TEXT("STATE: %s"), *AlertStateName),
				nullptr, StateColor, 3.f);
		}
	}
#endif
}

void ASAEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Actor->Implements<USADetectable>())
		{
			const bool bCanBeDetected = ISADetectable::Execute_CanBeDetected(Actor);
			if (bCanBeDetected)
			{
				HandleSightStimulus(Actor, Stimulus);
			}
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		HandleHearingStimulus(Actor, Stimulus);
	}
}

void ASAEnemyAIController::HandleSightStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		CurrentThreat           = Actor;
		LastKnownThreatLocation = Actor->GetActorLocation();

		SetAlertState(ESAAlertState::Alert);

		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->SetValueAsObject(SABlackboardKeys::ThreatActor,    Actor);
			BB->SetValueAsVector(SABlackboardKeys::ThreatLocation, LastKnownThreatLocation);
		}

		if (Actor->Implements<USADetectable>())
		{
			const ESADetectionProfile Profile = ISADetectable::Execute_GetDetectionProfile(Actor);
			ISADetectable::Execute_OnDetectedByEnemy(Actor, GetPawn(), Profile);
		}
	}
	else
	{
		CurrentThreat.Reset();

		if (CurrentAlertState == ESAAlertState::Alert)
		{
			SetAlertState(ESAAlertState::Searching);
		}

		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->ClearValue(SABlackboardKeys::ThreatActor);
		}
	}
}

void ASAEnemyAIController::HandleHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()) return;

	LastKnownThreatLocation = Stimulus.StimulusLocation;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsVector(SABlackboardKeys::ThreatLocation, LastKnownThreatLocation);
	}

	if (CurrentAlertState == ESAAlertState::Neutral)
	{
		SetAlertState(ESAAlertState::Suspicious);
	}
	else if (CurrentAlertState == ESAAlertState::Searching)
	{
		GetWorldTimerManager().ClearTimer(SearchTimerHandle);
		GetWorldTimerManager().SetTimer(SearchTimerHandle, this,
			&ASAEnemyAIController::OnSearchTimeout, AlertSearchDuration, false);
	}
}

void ASAEnemyAIController::OnSuspiciousTimeout()
{
	if (CurrentAlertState == ESAAlertState::Suspicious)
	{
		SetAlertState(ESAAlertState::Neutral);
	}
}

void ASAEnemyAIController::OnSearchTimeout()
{
	if (CurrentAlertState == ESAAlertState::Searching)
	{
		SetAlertState(ESAAlertState::Neutral);
	}
}