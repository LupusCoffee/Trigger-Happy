#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseMoveState.h"
#include "Systems/MovementSystem/CustomCharacterMovementComponent.h"
#include "CrouchState.generated.h"


class UCapsuleComponent;
class UCharacterMovementComponent;

UCLASS()
class GP4PROTOTYPE_API UCrouchState : public UBaseMoveState
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void Init(float InCrouchDownSpeed, float InUncrouchSpeed, float InitFallbackMoveSpeed, float CrouchHeightMultiplier);

	virtual bool Enter(FMovementContext Context) override;
	virtual bool Exit() override;
	virtual void Tick(float DeltaTime, FMovementContext Context) override;

protected:
	// methods
	UFUNCTION()
	void UpdateHeightLerp(float DeltaTime);

	UFUNCTION()
	void SetHeight(float TargetHeight);
	
	UFUNCTION()
	void Move(FVector2f MoveInput);
	
	// variables
	UPROPERTY()
	float CrouchDownSpeed;

	UPROPERTY()
	float UncrouchSpeed;

	UPROPERTY()
	float LerpProgress;

	UPROPERTY()
	bool bIsLerpingHeight;

	UPROPERTY()
	float LerpStartHeight;

	UPROPERTY()
	float LerpTargetHeight;

	UPROPERTY()
	bool bWantsToUncrouch;
	
	UPROPERTY()
	float FallbackMoveSpeed;

	UPROPERTY()
	float InitialCapsuleHalfHeight;

	UPROPERTY()
	float TargetCapsuleHalfHeight;

	UPROPERTY()
	float CurrentCapsuleHalfHeight;

	UPROPERTY()
	UCustomCharacterMovementComponent* CustomCharMoveComp;
	
	UPROPERTY()
	UCharacterMovementComponent* OwnerCharacterMovementComponent;
	
	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	UCameraComponent* OwnerCameraComp;

	UPROPERTY()
	UCapsuleComponent* OwnerCapsuleComponent;
	
	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;
};
