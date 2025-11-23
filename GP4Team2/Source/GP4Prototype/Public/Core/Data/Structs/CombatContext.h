#pragma once

#include "CombatContext.generated.h"

USTRUCT(BlueprintType)
struct FCombatContext
{
	GENERATED_BODY()


	// input context
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="input")
	bool bWantsToFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="input")
	bool bWantsToMelee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="input")
	bool bWantsToAbility;

	
	// combat values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="value")
	bool bHasNoFireCooldown;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="value")
	bool bFireChargeFullyDepleted;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="value")
	bool bCanMelee;
	

	FCombatContext() = default;
};
