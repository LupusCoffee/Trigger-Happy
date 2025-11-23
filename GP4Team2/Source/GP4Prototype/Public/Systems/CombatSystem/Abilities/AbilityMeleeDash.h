#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityObject.h"
#include "AbilityMeleeDash.generated.h"


UCLASS()
class GP4PROTOTYPE_API UAbilityMeleeDash : public UGameplayAbilityObject
{
	GENERATED_BODY()

public:
	virtual void Init_Implementation(UAbilityComponent* _MyAbilityComponent) override;
};
