#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatEventsSubsystem.generated.h"


UCLASS()
class GP4PROTOTYPE_API UCombatEventsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//--------------------------------------------------------------------------
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyKilled);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEnemyKilled OnEnemyKilled;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEnemyKilled OnEnemyKilledWithGun;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEnemyKilled OnEnemyKilledWithMelee;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEnemyKilled OnEnemyKilledWithExplosion;
	//--------------------------------------------------------------------------
	
	
	//--------------------------------------------------------------------------
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyHit);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEnemyHit OnEnemyHit;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEnemyHit OnEnemyHitWithGun;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEnemyHit OnEnemyHitWithMelee;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEnemyHit OnEnemyHitWithExplosion;
	//--------------------------------------------------------------------------
};
