#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseCombatState.h"
#include "Core/Subsystems/LookTraceSubsystem.h"
#include "MeleeState.generated.h"
class UAttributeComponent;
class UCombatFiniteStateMachine;
class UCombatComponent;

UCLASS()
class GP4PROTOTYPE_API UMeleeState : public UBaseCombatState
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void Init(float InitFallbackMeleeDamage,
	          float InitFallbackForwardMeleeKnockbackStrengthForSurvivingEnemies,
	          float InitUpwardMeleeKnockbackStrengthForSurvivingEnemies,
	          float InitFallbackForwardMeleeKnockbackStrengthForDyingEnemies, float InitMeleeForwardStartOffset, float InitMeleeRadius,
	          float InitMeleeReach, float InitMeleeWidth,
	          ECollisionChannel InitMeleeTraceChannel, float InitFallbackMaxMeleeHitCapacity,
	          float InitMeleeDuration, UAnimMontage* InitBatMeleeMontage, UStaticMeshComponent* InitBatMeshComp,
	          USkeletalMeshComponent* InitGunMeshComp, FName InBatBottomSocket, FName InBatTipSocket,
	          FName InMeleeStartNotify,
	          FName InMeleeEndNotify);
	virtual bool Enter(FCombatContext Context) override;
	virtual void Tick(float DeltaTime, FCombatContext Context) override;
	virtual bool Exit() override;

protected:
	// variables --> static values
	UPROPERTY()
	float FallbackMeleeDamage;

	UPROPERTY()
	float FallbackForwardMeleeKnockbackStrengthForSurvivingEnemies;

	UPROPERTY()
	float UpwardMeleeKnockbackStrengthForSurvivingEnemies;
	
	UPROPERTY()
	float FallbackForwardMeleeKnockbackStrengthForDyingEnemies;

	UPROPERTY()
	float MeleeForwardStartOffset;
	
	UPROPERTY()
	float MeleeRadius;

	UPROPERTY()
	float MeleeReach;

	UPROPERTY()
	float MeleeWidth;

	UPROPERTY()
	TEnumAsByte<ECollisionChannel> MeleeTraceChannel;

	UPROPERTY()
	int FallbackMaxMeleeHitCapacity;

	UPROPERTY()
	float MeleeDuration;

	UPROPERTY()
	UAnimMontage* BatMeleeMontage;

	UPROPERTY()
	UStaticMeshComponent* BatMeshComp;

	UPROPERTY()
	USkeletalMeshComponent* GunMeshComp;

	UPROPERTY()
	FName BatBottomSocket;

	UPROPERTY()
	FName BatTipSocket;

	UPROPERTY()
	FName MeleeStartNotify;

	UPROPERTY()
	FName MeleeEndNotify;

	
	// variables --> dynamic values
	UPROPERTY()
	float MeleeCurrentTime;

	UPROPERTY()
	float MeleeActiveStartTime;

	UPROPERTY()
	float MeleeActiveEndTime;

	UPROPERTY()
	bool bMeleeActive;
	
	UPROPERTY()
	float MeleeActiveDuration;

	UPROPERTY()
	float MeleeActiveCurrentTime;

	UPROPERTY()
	FVector PrevBase;
	
	UPROPERTY()
	FVector PrevTip;

	UPROPERTY()
	TArray<AActor*> DamagedActors;

	
	// variables --> components
	UPROPERTY()
	UCombatFiniteStateMachine* CombatFSM;

	UPROPERTY()
	AActor* OwnerActor;
	
	UPROPERTY()
	UCombatComponent* CombatComponent;

	UPROPERTY()
	ULookTraceSubsystem* LookTraceSubsystem;

	UPROPERTY()
	AController* Controller;

	UPROPERTY()
	UAnimInstance* AnimInstance;

	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;


	// methods
	UFUNCTION()
	void OnMeleeHit(FHitResult Hit);

	UFUNCTION()
	void KnockbackLiving(AActor* HitActor);

	UFUNCTION()
	void KnockbackDead(AActor* HitActor);
};
