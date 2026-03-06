// Copyright 2026, Lisboon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SACoverComponent.generated.h"

UENUM(BlueprintType)
enum class ECoverPeekSide : uint8
{
	None  UMETA(DisplayName = "None"),
	Left  UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STEALTHACTIONGAME_API USACoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USACoverComponent();

	UFUNCTION(BlueprintCallable, Category = "Cover")
	void ToggleCover();

	// === State Queries ===
	UFUNCTION(BlueprintPure, Category = "Cover")
	bool IsInCover() const { return bIsInCover; }

	UFUNCTION(BlueprintPure, Category = "Cover")
	ECoverPeekSide GetPeekSide() const { return CurrentPeekSide; }

	UFUNCTION(BlueprintPure, Category = "Cover")
	FVector GetCoverNormal() const { return CoverNormal; }

	UFUNCTION(BlueprintPure, Category = "Cover")
	FVector GetCoverTangent() const { return CoverTangent; }

	void UpdatePeekSide(const FVector& CameraRight);

	FVector2D FilterMovementInput(const FVector2D& RawInput, const FRotator& ControlRotation) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Detection",
		meta = (ClampMin = "50.0", ClampMax = "500.0"))
	float CoverDetectionRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Detection",
		meta = (ClampMin = "20.0", ClampMax = "150.0"))
	float CoverSnapDistance = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cover|Detection",
		meta = (ClampMin = "10.0", ClampMax = "100.0"))
	float DetectionSweepRadius = 30.f;

	// === Runtime State ===
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Cover|State")
	bool bIsInCover = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Cover|State")
	FVector CoverNormal = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Cover|State")
	FVector CoverTangent = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Cover|State")
	ECoverPeekSide CurrentPeekSide = ECoverPeekSide::None;

private:
	void TakeCover();

	void LeaveCover();
};