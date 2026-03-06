// Copyright 2026, Lisboon. All Rights Reserved.

#include "Components/SACoverComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#if ENABLE_DRAW_DEBUG
#include "DrawDebugHelpers.h"
#endif

USACoverComponent::USACoverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USACoverComponent::ToggleCover()
{
	if (bIsInCover)
		LeaveCover();
	else
		TakeCover();
}

void USACoverComponent::TakeCover()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector ForwardVector = Owner->GetActorForwardVector();
	if (const APawn* PawnOwner = Cast<APawn>(Owner))
	{
		if (const AController* PC = PawnOwner->GetController())
		{
			const FRotator ControlRot(0.f, PC->GetControlRotation().Yaw, 0.f);
			ForwardVector = ControlRot.Vector();
		}
	}

	const FVector Start = Owner->GetActorLocation();
	const FVector End   = Start + ForwardVector * CoverDetectionRange;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(DetectionSweepRadius);
	const bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult, Start, End, FQuat::Identity,
		ECC_Visibility, SweepShape, QueryParams
	);

	if (!bHit || !HitResult.GetActor())
	{
#if ENABLE_DRAW_DEBUG
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f, 0, 2.f);
#endif
		return; 
	}

	bIsInCover  = true;
	CoverNormal = HitResult.ImpactNormal;
	CoverTangent = FVector::CrossProduct(CoverNormal, FVector::UpVector).GetSafeNormal();

	FVector SnapPos = HitResult.ImpactPoint + CoverNormal * CoverSnapDistance;
	SnapPos.Z       = Owner->GetActorLocation().Z;
	const FRotator SnapRot = (-CoverNormal).Rotation();

	if (ACharacter* Char = Cast<ACharacter>(Owner))
	{
		Char->SetActorLocationAndRotation(SnapPos, SnapRot, true);

		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = false;
		}
	}

#if ENABLE_DRAW_DEBUG
	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.f, 0, 2.f);
	DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 25.f, 12, FColor::Cyan, false, 2.f);
	DrawDebugDirectionalArrow(GetWorld(), SnapPos, SnapPos + CoverNormal  * 80.f, 15.f, FColor::White,   false, 3.f, 0, 2.f);
	DrawDebugDirectionalArrow(GetWorld(), SnapPos, SnapPos + CoverTangent * 100.f, 15.f, FColor::Blue,  false, 3.f, 0, 2.f);
	DrawDebugDirectionalArrow(GetWorld(), SnapPos, SnapPos - CoverTangent * 100.f, 15.f, FColor::Blue,  false, 3.f, 0, 2.f);
	DrawDebugString(GetWorld(), SnapPos + FVector(0.f, 0.f, 100.f), TEXT("IN COVER"), nullptr, FColor::Cyan, 3.f);
#endif
}

void USACoverComponent::LeaveCover()
{
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = true;
		}
	}

	bIsInCover       = false;
	CoverNormal      = FVector::ZeroVector;
	CoverTangent     = FVector::ZeroVector;
	CurrentPeekSide  = ECoverPeekSide::None;
}

void USACoverComponent::UpdatePeekSide(const FVector& CameraRight)
{
	if (!bIsInCover)
	{
		CurrentPeekSide = ECoverPeekSide::None;
		return;
	}

	const float Dot = FVector::DotProduct(CameraRight, CoverTangent);
	CurrentPeekSide = (Dot >= 0.f) ? ECoverPeekSide::Right : ECoverPeekSide::Left;

#if ENABLE_DRAW_DEBUG
	if (const AActor* Owner = GetOwner())
	{
		const FVector ActorLoc = Owner->GetActorLocation();
		const FVector PeekDir  = (CurrentPeekSide == ECoverPeekSide::Right) ? CoverTangent : -CoverTangent;
		DrawDebugDirectionalArrow(GetWorld(), ActorLoc, ActorLoc + PeekDir * 120.f,
			20.f, FColor::Emerald, false, 0.5f, 0, 3.f);
	}
#endif
}

FVector2D USACoverComponent::FilterMovementInput(const FVector2D& RawInput, const FRotator& ControlRotation) const
{
	if (!bIsInCover) return RawInput;

	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector DesiredWorldDir = (ForwardDir * RawInput.Y + RightDir * RawInput.X).GetSafeNormal();
	const float   TangentDot      = FVector::DotProduct(DesiredWorldDir, CoverTangent);

	return FVector2D(TangentDot * RawInput.Size(), 0.f);
}