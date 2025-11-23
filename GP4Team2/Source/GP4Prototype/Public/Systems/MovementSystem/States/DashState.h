#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseMoveState.h"
#include "DashState.generated.h"
class UCapsuleComponent;
class UAttributeComponent;
class ULookTraceSubsystem;
class UCharacterMovementComponent;
class UCustomCharacterMovementComponent;
class UMovementFiniteStateMachine;


UCLASS()
class GP4PROTOTYPE_API UDashState : public UBaseMoveState
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void Init(float InputFallbackDashSpeed, float InputDashDuration, float InitAccelerationDuringDash, float InputDashFallSpeedInAir,
			  float InputDashGravityScale, bool InCanDashInAir, float InitVaultTraceLength, float InitVaultTraceRadius,
			  ECollisionChannel InitVaultCollisionChannel, float InitVaultHeightRequirement, FName InitRegularVaultableTagName, FName InitSlideVaultableTagName,
			  int InMaxDashHitCapacity, float InDashHitDamage, float InFallbackForwardHitKnockbackStrengthForSurvivingEnemies,
			  float InUpwardHitKnockbackStrengthForSurvivingEnemies, float InFallbackForwardHitKnockbackStrengthForDyingEnemies);

	virtual bool Enter(FMovementContext Context) override;
	virtual void Tick(float DeltaTime, FMovementContext Context) override;
	virtual bool Exit() override;

protected:
	UPROPERTY()
	float FallBackDashSpeed;

	UPROPERTY()
	float DashDuration;

	UPROPERTY()
	float AccelerationDuringDash;
	
	UPROPERTY()
	float DashFallSpeedInAir;

	UPROPERTY()
	float DashGravityScale;

	UPROPERTY()
	bool bCanDashInAir;

	
	// variables --> dynamic values
	UPROPERTY()
	FVector ResultDashDir;

	UPROPERTY()
	float InitialSpeed;

	UPROPERTY()
	float CurrentDashTime;

	UPROPERTY()
	float InitialAcceleration;

	UPROPERTY()
	float InitialDashFallSpeedInAir;

	UPROPERTY()
	float InitialDashGravityScale;
	

	// vault variables
	UPROPERTY()
	float VaultTraceLength;

	UPROPERTY()
	float VaultTraceRadius;

	UPROPERTY()
	TEnumAsByte<ECollisionChannel> VaultCollisionChannel;

	UPROPERTY()
	float VaultHeightRequirement;

	UPROPERTY()
	FName RegularVaultableTagName;

	UPROPERTY()
	FName SlideVaultableTagName;


	// variables --> components
	UPROPERTY()
	UMovementFiniteStateMachine* MovementFSM;

	UPROPERTY()
	AActor* OwnerActor;
	
	UPROPERTY()
	UAttributeComponent* AttributeComponent;
	
	UPROPERTY()
	UCustomCharacterMovementComponent* CustomCharMoveComp;

	UPROPERTY()
	UCharacterMovementComponent* CharMoveComp;

	UPROPERTY()
	UCapsuleComponent* OwnerCapsuleComponent;

	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	ULookTraceSubsystem* LookTraceSubsystem;

	// variables --> editable, melee dash stuff
	UPROPERTY()
	int MaxDashHitCapacity;

	UPROPERTY()
	float FallbackDashHitDamage;

	UPROPERTY()
	float FallbackForwardHitKnockbackStrengthForSurvivingEnemies;

	UPROPERTY()
	float UpwardHitKnockbackStrengthForSurvivingEnemies;
	
	UPROPERTY()
	float FallbackForwardHitKnockbackStrengthForDyingEnemies;

	
	// variables --> hidden, melee dash stuff
	UPROPERTY()
	TArray<AActor*> DamagedActors;


	// methods
	UFUNCTION()
	bool StartDash(FVector2f MoveInput);
	
	UFUNCTION()
	void Dash();

	UFUNCTION()
	bool StopDash();

	// methods --> extra stuff
	UFUNCTION()
	void TryDashHit();
	UFUNCTION()
	void KnockbackLiving(AActor* HitActor);
	UFUNCTION()
	void KnockbackDead(AActor* HitActor);

	bool TryVaultTransition(FMovementContext Context);
};
