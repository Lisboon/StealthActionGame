// Copyright 2026, Lisboon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SACharacterMovementComponent.generated.h"

UCLASS()
class STEALTHACTIONGAME_API USACharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// === Speed Config ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed",
		meta = (ClampMin = "100.0", ClampMax = "1200.0"))
	float WalkSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed",
		meta = (ClampMin = "100.0", ClampMax = "1200.0"))
	float RunSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed",
		meta = (ClampMin = "50.0", ClampMax = "400.0"))
	float CrouchSpeed = 150.f;

	// === Sprint Intent ===
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State")
	bool bWantsToRun = false;

	// === Core Override ===
	virtual float GetMaxSpeed() const override;
};