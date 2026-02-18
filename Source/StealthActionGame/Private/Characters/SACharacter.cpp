// Copyright 2026, Lisboon. All Rights Reserved.

#include "Characters/SACharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"

ASACharacter::ASACharacter()
{
	// === Camera Components ===
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 300.0f;
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	
	// === Default Movement Config ===
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
	UpdateMovementSpeed();
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

void ASACharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = WalkSpeed;
		Movement->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

void ASACharacter::Move(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();
	
	if (!Controller || InputVector.IsNearlyZero())
		return;
	
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	
	const FRotationMatrix RotMatrix(YawRotation);
	
	const FVector ForwardDir = RotMatrix.GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDir, InputVector.Y);
	
	const FVector RightDir   = RotMatrix.GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDir, InputVector.X);
}

void ASACharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookInput = Value.Get<FVector2D>();
	
	if (!Controller || LookInput.IsNearlyZero())
		return;
	
	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

// === Logic Run In Development ===
void ASACharacter::StartRun()
{
	bIsRunning = true;
	UpdateMovementSpeed();
}

void ASACharacter::StopRun()
{
	bIsRunning = false;
	UpdateMovementSpeed();
}

// === Logic Crouch In Development ===
void ASACharacter::StartCrouch()
{
	Crouch();
}

void ASACharacter::StopCrouch()
{
	UnCrouch();
}

// === Logic Jump In Development ===
void ASACharacter::StartJump()
{
	Jump();
}

void ASACharacter::StopJump()
{
	StopJumping();
}

void ASACharacter::UpdateMovementSpeed()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	
	if (!Movement) return;

	Movement->MaxWalkSpeed = bIsRunning ? RunSpeed : WalkSpeed;
}
