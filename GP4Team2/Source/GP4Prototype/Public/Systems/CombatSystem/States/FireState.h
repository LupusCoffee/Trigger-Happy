#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseCombatState.h"
#include "Core/Subsystems/LookTraceSubsystem.h"
#include "Systems/CombatSystem/CombatComponent.h"
#include "FireState.generated.h"
class UCombatFiniteStateMachine;


UCLASS()
class GP4PROTOTYPE_API UFireState : public UBaseCombatState
{
	GENERATED_BODY()

public:
	void Init(float InitDamage, float InitFireRate, float InitFireTraceLength, float InitFireTraceRadius,
			  USkeletalMeshComponent* InitCharArms, UAnimMontage* InitGunFireMontage);
	virtual bool Enter(FCombatContext Context) override;
	virtual void Tick(float DeltaTime, FCombatContext Context) override;

protected:
	// variables --> static values
	UPROPERTY()
	float GunDamage;

	UPROPERTY()
	float FireRate;

	UPROPERTY()
	float FireTraceLength;

	UPROPERTY() //todo: change to spread (don't wanna sphere cast)
	float FireTraceRadius;

	UPROPERTY()
	USkeletalMeshComponent* CharArms = nullptr;

	UPROPERTY()
	UAnimMontage* GunFireMontage = nullptr;
	

	// variables --> dynamic values
	UPROPERTY()
	float FireTimer = 0;

	// variables --> components
	UPROPERTY()
	UCombatFiniteStateMachine* CombatFSM;

	UPROPERTY()
	ULookTraceSubsystem* LookTraceSubsystem;
	
	UPROPERTY()
	AController* Controller;

	UPROPERTY()
	UCombatComponent* CombatComponent;

	UPROPERTY()
	UAnimInstance* AnimInstance;

	
	// methods
	UFUNCTION()
	void Fire();
};
