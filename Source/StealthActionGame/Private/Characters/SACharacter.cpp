// Copyright 2026, Lisboon. All Rights Reserved.

#include "Characters/SACharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Characters/SACharacterMovementComponent.h"

ASACharacter::ASACharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<USACharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	SAMovementComponent = Cast<USACharacterMovementComponent>(GetSACharacterMovement());
	
	// === Camera Components ===
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 300.0f;
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	
	// === Movement ===
	bUseControllerRotationYaw = false;
	
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
		Movement->GetNavAgentPropertiesRef().bCanCrouch = true;
	}
}

void ASACharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if  (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}

void ASACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASACharacter::Move);
		
		if (LookAction)
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASACharacter::Look);
		
		if (RunAction)
		{
			EnhancedInput->BindAction(RunAction, ETriggerEvent::Started, this, &ASACharacter::StartRun);
			EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &ASACharacter::StopRun);
		}
		
		if (CrouchAction)
		{
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASACharacter::StartCrouch);
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ASACharacter::StopCrouch);
		}
		
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ASACharacter::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASACharacter::StopJump);
		}
	}
}

void ASACharacter::StartCrouch()
{
	Crouch();
}

void ASACharacter::StopCrouch()
{
	UnCrouch();
}

void ASACharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CurrentStance = EMovementStance::Crouching;

	if (SAMovementComponent)
	{
		SAMovementComponent->bWantsToRun = false;
	}
}

void ASACharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CurrentStance = EMovementStance::Standing;
}

void ASACharacter::StartRun()
{
	if (CurrentStance == EMovementStance::Crouching)
		return;

	if (SAMovementComponent)
	{
		SAMovementComponent->bWantsToRun = true;
	}
}

void ASACharacter::StopRun()
{
	if (SAMovementComponent)
	{
		SAMovementComponent->bWantsToRun = false;
	}
}

void ASACharacter::StartJump()
{
	Jump();
}

void ASACharacter::StopJump()
{
	StopJumping();
}

void ASACharacter::Move(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();

	if (!Controller || InputVector.IsNearlyZero())
		return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FRotationMatrix RotMatrix(YawRotation);

	AddMovementInput(RotMatrix.GetUnitAxis(EAxis::X), InputVector.Y);
	AddMovementInput(RotMatrix.GetUnitAxis(EAxis::Y), InputVector.X);
}

void ASACharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	if (!Controller || LookInput.IsNearlyZero())
		return;

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}