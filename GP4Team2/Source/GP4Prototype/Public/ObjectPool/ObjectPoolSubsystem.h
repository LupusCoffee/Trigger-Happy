// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

/**
 * 
*/
UCLASS()
class GP4PROTOTYPE_API UObjectPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Registers (or gets existing) pool for the given class */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void RegisterPool(TSubclassOf<AActor> ActorClass, int32 InitialSize = 10);

	/** Get (and activate) an actor from the appropriate pool */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	AActor* AcquireActor(TSubclassOf<AActor> ActorClass);

	/** Return an actor back to its pool */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReleaseActor(AActor* Actor);

	// Debug options
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugOnScreen = false;

	UPROPERTY(EditAnywhere, Category = "Debug", meta=(ClampMin="0.0"))
	float DebugDuration = 5.0f;

private:
	/** Mapping between Actor class and its pool instance */
	UPROPERTY()
	TMap<TSubclassOf<AActor>, TObjectPtr<UObjectPoolBase>> Pools;

	UObjectPoolBase* GetOrCreatePool(TSubclassOf<AActor> ActorClass, int32 DefaultSize = 10);
};