#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseMoveState.h"
#include "SprintState.generated.h"


class UCapsuleComponent;
class UAttributeComponent;
class UCustomCharacterMovementComponent;
class ULookTraceSubsystem;
class UCharacterMovementComponent;

UCLASS()
class GP4PROTOTYPE_API USprintState : public UBaseMoveState
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void Init(float InitFallbackMoveSpeed, float InitVaultTraceLength, float InitVaultTraceRadius, ECollisionChannel InitVaultTraceChannel,
			  float InitVaultHeightRequirement, FName InitRegularVaultableTagName, FName InitSlideVaultableTagName);

	virtual bool Enter(FMovementContext Context) override;
	virtual void Tick(float DeltaTime, FMovementContext Context) override;
	virtual bool Exit() override;

protected:
	// methods
	UFUNCTION()
	void Move(FVector2f MoveInput);

	UFUNCTION()
	bool TryVaultTransition(FMovementContext Context);
	
	// variables
	UPROPERTY()
	float FallbackMoveSpeed;

	// vault variables
	UPROPERTY()
	float VaultTraceLength;

	UPROPERTY()
	float VaultTraceRadius;

	UPROPERTY()
	TEnumAsByte<ECollisionChannel> VaultTraceChannel;

	UPROPERTY()
	float VaultHeightRequirement;

	UPROPERTY()
	FName RegularVaultableTagName;
	
	UPROPERTY()
	FName SlideVaultableTagName;

	UPROPERTY()
	UCustomCharacterMovementComponent* CustomCharMoveComp;

	UPROPERTY()
	UCharacterMovementComponent* OwnerCharacterMovementComponent;
	
	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	UCapsuleComponent* OwnerCapsuleComponent;

	UPROPERTY()
	ULookTraceSubsystem* LookTraceSubsystem;

	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;
};
