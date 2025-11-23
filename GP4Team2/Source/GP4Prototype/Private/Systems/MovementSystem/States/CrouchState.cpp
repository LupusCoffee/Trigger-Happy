#include "Systems/MovementSystem/States/CrouchState.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Systems/CombatSystem/States/IdleState.h"
#include "Systems/MovementSystem/MovementFiniteStateMachine.h"
#include "Systems/MovementSystem/States/DashState.h"
#include "Systems/MovementSystem/States/SprintState.h"
#include "Systems/MovementSystem/States/WalkState.h"

void UCrouchState::Init(float InCrouchDownSpeed, float InUncrouchSpeed, float InitFallbackMoveSpeed, float CrouchHeightMultiplier)
{
	UMovementFiniteStateMachine* MovementFSM = Cast<UMovementFiniteStateMachine>(FSM);
	if (!MovementFSM) return;
	
	CustomCharMoveComp = MovementFSM->GetOwnerCustomCharacterMovementComponent();

	// set values
	CrouchDownSpeed = InCrouchDownSpeed;
	UncrouchSpeed = InUncrouchSpeed;
	
	FallbackMoveSpeed = InitFallbackMoveSpeed;

	
	if (AActor* OwnerActor = MovementFSM->GetOwnerActor())
	{
		OwnerCharacterMovementComponent = OwnerActor->GetComponentByClass<UCharacterMovementComponent>();

		
		OwnerCapsuleComponent = OwnerActor->GetComponentByClass<UCapsuleComponent>();
		if (OwnerCapsuleComponent)
		{
			InitialCapsuleHalfHeight = OwnerCapsuleComponent->GetUnscaledCapsuleHalfHeight();
			CurrentCapsuleHalfHeight = OwnerCapsuleComponent->GetUnscaledCapsuleHalfHeight();
			TargetCapsuleHalfHeight = InitialCapsuleHalfHeight * CrouchHeightMultiplier;
		}
	}

	
	OwnerPawn = MovementFSM->GetOwnerPawn();
	if (OwnerPawn) AttributeComponent = OwnerPawn->GetComponentByClass<UAttributeComponent>();
}

bool UCrouchState::Enter(FMovementContext Context)
{
	//set character movement component max speed, acceleration, etc.
	float MoveSpeed = FallbackMoveSpeed;
	if (AttributeComponent) MoveSpeed = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Movement_CrouchSpeed);
	
	if (OwnerCharacterMovementComponent) OwnerCharacterMovementComponent->MaxWalkSpeed = MoveSpeed;

	// Read the real capsule height so we start from the actual current height (e.g. after slide)
	if (OwnerCapsuleComponent)
	{
		CurrentCapsuleHalfHeight = OwnerCapsuleComponent->GetUnscaledCapsuleHalfHeight();
	}

	// Setup lerp targets
	bWantsToUncrouch = false;
	LerpStartHeight = CurrentCapsuleHalfHeight;
	LerpTargetHeight = TargetCapsuleHalfHeight;

	// If we're already at/near crouch height, skip lerp entirely
	const float HeightEpsilon = 0.5f;
	if (FMath::IsNearlyEqual(LerpStartHeight, LerpTargetHeight, HeightEpsilon))
	{
		bIsLerpingHeight = false;
		LerpProgress = 0.0f;
		SetHeight(LerpTargetHeight);
	}
	else
	{
		// Start lerping down to crouch height
		bIsLerpingHeight = true;
		LerpProgress = 0.0f;
	}

	CustomCharMoveComp->OnCrouchStart.Broadcast();
	
	return true;
}

bool UCrouchState::Exit()
{
	// Return to initial height using the same anchored method
	SetHeight(InitialCapsuleHalfHeight);

	CustomCharMoveComp->OnCrouchFinish.Broadcast();
	
	return true;
}

void UCrouchState::Tick(float DeltaTime, FMovementContext Context)
{
	Super::Tick(DeltaTime, Context);
	
	Move(Context.MoveInput);

	// Handle uncrouch request
	if (!Context.bWantsToCrouch && !bWantsToUncrouch)
	{
		// Start lerping back up
		bWantsToUncrouch = true;
		bIsLerpingHeight = true;
		LerpProgress = 0.0f;
		LerpStartHeight = CurrentCapsuleHalfHeight;
		LerpTargetHeight = InitialCapsuleHalfHeight;
	}

	// Update height lerping
	if (bIsLerpingHeight)
	{
		UpdateHeightLerp(DeltaTime);
	}

	// Only transition to walk state after uncrouch lerp is complete
	if (bWantsToUncrouch && !bIsLerpingHeight)
	{
		FSM->SetState(UWalkState::StaticClass(), Context);
	}
}

void UCrouchState::UpdateHeightLerp(float DeltaTime)
{
	LerpProgress += DeltaTime;
	
	// Use different speeds for crouch vs uncrouch
	float CurrentLerpSpeed = bWantsToUncrouch ? UncrouchSpeed : CrouchDownSpeed;
	float Alpha = FMath::Clamp(LerpProgress / CurrentLerpSpeed, 0.0f, 1.0f);
	float NewHeight = FMath::Lerp(LerpStartHeight, LerpTargetHeight, Alpha);
	
	SetHeight(NewHeight);

	// Check if lerp is complete - use a small epsilon to prevent snapping
	if (Alpha >= 1.0f)
	{
		// Ensure we set the exact target height to prevent floating point errors
		SetHeight(LerpTargetHeight);
		bIsLerpingHeight = false;
		LerpProgress = 0.0f;
	}
}

void UCrouchState::SetHeight(float TargetHeight)
{
	if (!OwnerCapsuleComponent || !OwnerPawn) return;

	// Cache previous height before change
	const float PrevHalfHeight = CurrentCapsuleHalfHeight;

	// Apply new capsule half height
	OwnerCapsuleComponent->SetCapsuleHalfHeight(TargetHeight);
	CurrentCapsuleHalfHeight = OwnerCapsuleComponent->GetUnscaledCapsuleHalfHeight();

	// Compute per-step delta to keep feet planted:
	// Bottom should remain constant => move center DOWN by (Prev - New)
	const float DeltaHalf = PrevHalfHeight - CurrentCapsuleHalfHeight;
	if (!FMath::IsNearlyZero(DeltaHalf))
	{
		const FVector DownOffset = -OwnerPawn->GetActorUpVector() * DeltaHalf;
		FHitResult SweepHit;
		OwnerPawn->AddActorWorldOffset(DownOffset, true, &SweepHit);
		// Optional: if sweep blocked upward uncrouch, you could clamp here (not needed now)
	}
}

void UCrouchState::Move(FVector2f MoveInput)
{
	if (!OwnerPawn) return;
	
	FVector ForwardBack = OwnerPawn->GetActorForwardVector() * MoveInput.Y;
	FVector LeftRight = OwnerPawn->GetActorRightVector() * MoveInput.X;
	FVector ResultMoveDir = (ForwardBack+LeftRight).GetSafeNormal();
	
	OwnerPawn->AddMovementInput(ResultMoveDir);
}
