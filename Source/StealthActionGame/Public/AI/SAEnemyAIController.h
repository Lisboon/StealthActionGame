// Copyright 2026, Lisboon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AI/SAAlertState.h"
#include "SAEnemyAIController.generated.h"

class UAIPerceptionComponent;
class UBehaviorTree;

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