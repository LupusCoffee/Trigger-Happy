#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityObject.h"
#include "AbilityResurrect.generated.h"


UCLASS()
class GP4PROTOTYPE_API UAbilityResurrect : public UGameplayAbilityObject
{
	GENERATED_BODY()

public:
	virtual void Init_Implementation(UAbilityComponent* _MyAbilityComponent) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ability")
	float PercentageOfMaxHealthToReplenish = 0.5f;
};
