#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityObject.h"
#include "AbilityMeleeSlide.generated.h"


UCLASS()
class GP4PROTOTYPE_API UAbilityMeleeSlide : public UGameplayAbilityObject
{
	GENERATED_BODY()

public:
	virtual void Init_Implementation(UAbilityComponent* _MyAbilityComponent) override;
};
