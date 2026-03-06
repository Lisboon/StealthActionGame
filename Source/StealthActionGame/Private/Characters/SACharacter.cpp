// Copyright 2026, Lisboon. All Rights Reserved.

#include "Characters/SACharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Characters/SACharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"

ASACharacter::ASACharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USACharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	SAMovementComponent = Cast<USACharacterMovementComponent>(GetCharacterMovement());

	// === Camera ===
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 300.f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	// === Movement ===
	bUseControllerRotationYaw = false;

	if (SAMovementComponent)
	{
		SAMovementComponent->bOrientRotationToMovement = true;
		SAMovementComponent->GetNavAgentPropertiesRef().bCanCrouch = true;
	}
}

void ASACharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
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

	if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
			EI->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASACharacter::Move);

		if (LookAction)
			EI->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASACharacter::Look);

		if (RunAction)
		{
			EI->BindAction(RunAction, ETriggerEvent::Started,   this, &ASACharacter::StartRun);
			EI->BindAction(RunAction, ETriggerEvent::Completed, this, &ASACharacter::StopRun);
		}

		if (CrouchAction)
		{
			EI->BindAction(CrouchAction, ETriggerEvent::Started,   this, &ASACharacter::StartCrouch);
			EI->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ASACharacter::StopCrouch);
		}

		if (JumpAction)
		{
			EI->BindAction(JumpAction, ETriggerEvent::Started,   this, &ASACharacter::StartJump);
			EI->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASACharacter::StopJump);
		}

		if (CoverAction && CoverComponent)
		{
			EI->BindAction(CoverAction, ETriggerEvent::Started, CoverComponent.Get(), &USACoverComponent::ToggleCover);
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

bool ASACharacter::IsSprinting() const
{
	return SAMovementComponent && SAMovementComponent->bWantsToRun;
}

void ASACharacter::Move(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();

	if (!Controller || InputVector.IsNearlyZero())
		return;

	if (CoverComponent && CoverComponent->IsInCover())
    {
        InputVector = CoverComponent->FilterMovementInput(InputVector, Controller->GetControlRotation());

        if (InputVector.IsNearlyZero())
            return;

        const FRotationMatrix CamMatrix(Controller->GetControlRotation());
        CoverComponent->UpdatePeekSide(CamMatrix.GetUnitAxis(EAxis::Y));
    }

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