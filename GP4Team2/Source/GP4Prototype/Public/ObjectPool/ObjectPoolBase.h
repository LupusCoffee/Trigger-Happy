// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolTemplate.h"
#include "UObject/Object.h"
#include "ObjectPoolBase.generated.h"

/**
 * 
*/
UCLASS(Blueprintable)
class GP4PROTOTYPE_API UObjectPoolBase : public UObject
{
	GENERATED_BODY()
	
	virtual void BeginDestroy() override;

public:
	UFUNCTION(BlueprintCallable)
	void InitializePool(TSubclassOf<AActor> InClass, int32 InSize);

	UFUNCTION(BlueprintCallable)
	AActor* AcquireActor();

	UFUNCTION(BlueprintCallable)
	void ReleaseActor(AActor* Actor);

	// Expose pooled class for robust pool lookup
	UFUNCTION(BlueprintCallable)
	TSubclassOf<AActor> GetPooledClass() const { return PooledClass; }

private:
	void SyncGCProtection();
	void RebuildPoolForWorldChange();

	// Grow the pool by spawning AdditionalCount actors and returning them into the pool
	bool GrowPool(int32 AdditionalCount);
	
	TSubclassOf<AActor> PooledClass;
	TObjectPool<AActor>* PoolInstance = nullptr;
	int32 InitialSizeCached = 0;
	TWeakObjectPtr<UWorld> CachedWorld; // weak to avoid dangling after world teardown

	// This UPROPERTY array prevents the pooled actors from being garbage collected
	UPROPERTY()
	TArray<TObjectPtr<AActor>> GCProtectedPool;
};