// Copyright 2026, Lisboon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AI/SAAlertState.h"
#include "SAEnemyAIController.generated.h"

class UAIPerceptionComponent;
class UBehaviorTree;

// Static Blackboard key names.
// Defined here so BTTasks, BTDecorators and this controller all reference
// the same strings. A typo in a FName("...") literal is a silent runtime bug.
namespace SABlackboardKeys
{
	static const FName AlertState     = TEXT("AlertState");
	static const FName ThreatActor    = TEXT("ThreatActor");
	static const FName ThreatLocation = TEXT("ThreatLocation");
}

UCLASS()
class STEALTHACTIONGAME_API ASAEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASAEnemyAIController();

	UFUNCTION(BlueprintPure, Category = "AI|State")
	ESAAlertState GetAlertState() const { return CurrentAlertState; }

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetAlertState(ESAAlertState NewState);

	UFUNCTION(BlueprintPure, Category = "AI|Perception")
	FVector GetLastKnownThreatLocation() const { return LastKnownThreatLocation; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception")
	AActor* GetCurrentThreat() const { return CurrentThreat; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception")
	bool HasCurrentThreat() const { return IsValid(CurrentThreat); }

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Behavior")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	// === Perception Tuning ===
	// NOTE: These are read in the constructor to configure SenseConfigs.
	// Changing them in a derived Blueprint will NOT affect the SenseConfig
	// unless PostInitializeComponents re-applies them via RequestStimuliListenerUpdate().
	// For now, treat these as CDO-level defaults only.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception|Sight")
	float SightRadius = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception|Sight")
	float LoseSightRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception|Sight")
	float PeripheralVisionAngle = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception|Hearing")
	float HearingRange = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|State")
	float SuspiciousTimeout = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|State")
	float AlertSearchDuration = 10.f;

private:
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void HandleSightStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	void HandleHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	// UFUNCTION required — SetTimer with a member pointer needs UObject reflection.
	// Without this, the timer delegate is invalid and callbacks never fire.
	UFUNCTION()
	void OnSuspiciousTimeout();

	UFUNCTION()
	void OnSearchTimeout();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|State", meta = (AllowPrivateAccess = "true"))
	ESAAlertState CurrentAlertState = ESAAlertState::Neutral;

	UPROPERTY()
	TObjectPtr<AActor> CurrentThreat;

	FVector LastKnownThreatLocation = FVector::ZeroVector;

	FTimerHandle SuspiciousTimerHandle;
	FTimerHandle SearchTimerHandle;
};