#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseMoveState.h"
#include "SlideVaultState.generated.h"


class ULookTraceSubsystem;
class UCapsuleComponent;
class APawn;
class UCustomCharacterMovementComponent;
class UCharacterMovementComponent;
struct FMovementContext;

UCLASS()
class GP4PROTOTYPE_API USlideVaultState : public UBaseMoveState
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void Init(float InitSlideStrength, float InitSlideFrictionLevel, float InitSlideHeightMultiplier, FName InitVaultableTagName,
			  ECollisionChannel InitVaultTraceChannel);

	virtual bool Enter(FMovementContext Context) override;
	virtual bool Exit() override;
	virtual void Tick(float DeltaTime, FMovementContext Context) override;

protected:
	// methods
	UFUNCTION()
	bool StartSlide(FVector SlideStartLocation);

	UFUNCTION()
	bool StopSlide();

	UFUNCTION()
	bool TryWalkTransition(FMovementContext Context);
	
	// variables
	UPROPERTY()
	float SlideStrength;
	
	UPROPERTY()
	float SlideFrictionLevel;

	UPROPERTY()
	FName VaultableTagName;


	//ground checks
	TEnumAsByte<ECollisionChannel> VaultTraceChannel;


	UPROPERTY()
	float InitialCapsuleHalfHeight;

	UPROPERTY()
	float InitialCameraHeight;

	UPROPERTY()
	float CurrentCapsuleHalfHeight;

	UPROPERTY()
	float CurrentCameraHeight;

	UPROPERTY()
	float GroundOffset;
	
	
	UPROPERTY()
	float InitialGroundFrictionLevel;

	UPROPERTY()
	float InitialBrakingFrictionLevel;
	
	UPROPERTY()
	float InitialBreakingFrictionFactor;

	UPROPERTY()
	float InitialDecelarationWalking;
	
	UPROPERTY()
	bool InitialValueOnSeperateBrakingFriction;
	
	
	UPROPERTY()
	UCharacterMovementComponent* OwnerCharacterMovementComponent;

	UPROPERTY()
	UCustomCharacterMovementComponent* CustomCharMoveComp;
	
	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	UCapsuleComponent* OwnerCapsuleComponent;
	
	UPROPERTY()
	ULookTraceSubsystem* LookTraceSubsystem;
};