#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityObject.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySlowMo.generated.h"

class UAttributeComponent;

UCLASS()
class GP4PROTOTYPE_API UAbilitySlowMo : public UGameplayAbilityObject
{
	GENERATED_BODY()

public:
	virtual void Init_Implementation(UAbilityComponent* _MyAbilityComponent) override;
	virtual bool StartUsing() override;
	virtual void StopUsing() override;

protected:
	// variables --> editable
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Abilities --> fallback settings")
	float FallbackTimeDilationStrength = 0.05;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Abilities --> fallback settings")
	float FallbackTimeDilationDuration = 2.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Abilities --> fallback settings")
	float FallbackSlowMoCooldown = 5.0f;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Abilities")
	float PlayerSpeedMultiplierDuringSlowMo = 2.5f;*/
	
	// variables --> hidden, components
	UPROPERTY()
	UWorld* World = nullptr;

	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;
	
	/*UPROPERTY()
	UCharacterMovementComponent* CharMoveComp = nullptr;*/
};
