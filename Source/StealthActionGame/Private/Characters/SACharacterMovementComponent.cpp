// Copyright 2026, Lisboon. All Rights Reserved.

#include "Characters/SACharacterMovementComponent.h"

float USACharacterMovementComponent::GetMaxSpeed() const
{
	if (IsCrouching())
		return CrouchSpeed;

	if (bWantsToRun && MovementMode == MOVE_Walking)
		return RunSpeed;

	return WalkSpeed;
}