#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"
enum class EGameDamageType : uint8;


UINTERFACE(Blueprintable)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class GP4PROTOTYPE_API IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void TakeDamage(EGameDamageType DamageType, float DamageValue, float DamageMultiplier, FName HitSocketName);
};
