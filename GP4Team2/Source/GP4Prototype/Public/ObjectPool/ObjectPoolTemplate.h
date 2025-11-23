#pragma once

#include "CoreMinimal.h"

template <typename T>
class TObjectPool
{
public:
	TObjectPool(UWorld* InWorld, TSubclassOf<T> InClass, int32 InInitialSize, UObject* Owner);
	T* Acquire();
	void Release(T* Obj);

	TArray<TWeakObjectPtr<T>>& GetPool() { return Pool; }

	void RefreshWorld(UWorld* InWorld) { if (InWorld && InWorld != WorldPtr.Get()) { WorldPtr = InWorld; } }

private:
	T* SpawnNew();
	// Stored as weak to avoid dangling after world teardown
	TWeakObjectPtr<UWorld> WorldPtr;
	TSubclassOf<T> ClassToSpawn;
	TArray<TWeakObjectPtr<T>> Pool;
};

#include "ObjectPool/ObjectPoolTemplate.inl"
