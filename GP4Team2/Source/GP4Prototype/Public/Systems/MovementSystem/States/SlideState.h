#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseMoveState.h"
#include "SlideState.generated.h"


class ULookTraceSubsystem;
class UAttributeComponent;
class UCustomCharacterMovementComponent;
class UCharacterMovementComponent;
class UCapsuleComponent;
class UMovementFiniteStateMachine;

UCLASS()
class GP4PROTOTYPE_API USlideState : public UBaseMoveState
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void Init(float InitFallbackSlideStrength, float InitSlideFrictionLevel, float InitFallbackSlideDuration, float InitSlideHeightMultiplier,
	int InMaxDashHitCapacity, float InDashHitDamage, float InFallbackForwardHitKnockbackStrengthForSurvivingEnemies,
	float InUpwardHitKnockbackStrengthForSurvivingEnemies, float InFallbackForwardHitKnockbackStrengthForDyingEnemies,
		  float InitVaultTraceLength, float InitVaultTraceRadius);

	virtual bool Enter(FMovementContext Context) override;
	virtual bool Exit() override;
	virtual void Tick(float DeltaTime, FMovementContext Context) override;

protected:
	// methods
	UFUNCTION()
	bool StartSlide();

	UFUNCTION()
	bool StopSlide();

	// variables
	UPROPERTY()
	float FallbackSlideStrength;
	
	UPROPERTY()
	float SlideFrictionLevel;

	UPROPERTY()
	float FallbackSlideDuration;
	
	
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
	float InitialFrictionLevel;
		
	UPROPERTY()
	float CurrentSlideDuration;

	
	UPROPERTY()
	UCharacterMovementComponent* OwnerCharacterMovementComponent;

	UPROPERTY()
	UCustomCharacterMovementComponent* CustomCharMoveComp;
	
	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	UCapsuleComponent* OwnerCapsuleComponent;

	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;

	UPROPERTY()
	ULookTraceSubsystem* LookTraceSubsystem;


	// variables --> editable, melee dash stuff
	UPROPERTY()
	int MaxDashHitCapacity;

	UPROPERTY()
	float FallbackSlideHitDamage;

	UPROPERTY()
	float FallbackForwardHitKnockbackStrengthForSurvivingEnemies;

	UPROPERTY()
	float UpwardHitKnockbackStrengthForSurvivingEnemies;
	
	UPROPERTY()
	float FallbackForwardHitKnockbackStrengthForDyingEnemies;

	UPROPERTY()
	float VaultTraceLength;

	UPROPERTY()
	float VaultTraceRadius;

	// Preserve lowered capsule height on exit when transitioning directly to Crouch
	UPROPERTY()
	bool bPreserveLoweredHeightOnExit = false;

	
	// variables --> hidden, melee dash stuff
	UPROPERTY()
	TArray<AActor*> DamagedActors;


	// methods --> melee stuff
	UFUNCTION()
	void TryDashHit();
	
	UFUNCTION()
	void KnockbackLiving(AActor* HitActor);

	UFUNCTION()
	void KnockbackDead(AActor* HitActor);
};
