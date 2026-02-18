// Copyright 2026, Lisboon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SACharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class STEALTHACTIONGAME_API ASACharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASACharacter();

protected:
	// === Default Cycle ===
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

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
	
	// === Movement Speed ===
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed")
	float WalkSpeed = 300.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed")
	float RunSpeed = 600.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed")
	float CrouchSpeed = 150.f;
	
	// === Movement State ===
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement|State")
	bool bIsRunning = false;
	
	// === Input Handlers ===
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void StartRun();
	void StopRun();
	
	void StartCrouch();
	void StopCrouch();
	
	void StartJump();
	void StopJump();
	
	// Atualiza velocidade maxWalkSpeed conforme estado atual
	void UpdateMovementSpeed();
};
