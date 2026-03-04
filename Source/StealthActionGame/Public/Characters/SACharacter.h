// Copyright 2026, Lisboon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SACharacterMovementComponent.h"
#include "SACharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;
class USACharacterMovementComponent;

UENUM(BlueprintType)
enum class EMovementStance : uint8
{
	Standing	UMETA(DisplayName = "Standing"),
	Crouching	UMETA(DisplayName = "Crouching")
};

UCLASS()
class STEALTHACTIONGAME_API ASACharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASACharacter(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintPure, Category = "Movement")
	USACharacterMovementComponent* GetSACharacterMovement() const { return SAMovementComponent; }

protected:
	// === Default Cycle ===
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// === Camera Components ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;
	
	// === Input Components ===
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RunAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	
	// === Movement State ===
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State")
	EMovementStance CurrentStance = EMovementStance::Standing;
	
private:

	UPROPERTY()
	TObjectPtr<USACharacterMovementComponent> SAMovementComponent;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void StartRun();
	void StopRun();
	
	void StartCrouch();
	void StopCrouch();
	
	void StartJump();
	void StopJump();
};
