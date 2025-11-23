#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityObject.h"
#include "AbilityShield.generated.h"


UCLASS()
class GP4PROTOTYPE_API UAbilityShield : public UGameplayAbilityObject
{
	GENERATED_BODY()

public:
	virtual void Init_Implementation(UAbilityComponent* _MyAbilityComponent) override;

	//maybe i should tick the actual values here and not in health comp... hmm... yeah.............

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ability")
	float MaxShieldValue = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ability")
	float TimeBeforeShieldRecharge = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ability")
	float ShieldRechargeRate = 1.0f;
};
