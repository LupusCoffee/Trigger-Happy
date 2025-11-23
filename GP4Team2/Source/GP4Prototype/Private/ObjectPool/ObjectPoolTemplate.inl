// ObjectPoolTemplate.inl
#pragma once

#include "Debug.h"

template <typename T>
TObjectPool<T>::TObjectPool(UWorld* InWorld, TSubclassOf<T> InClass, int32 InInitialSize, UObject* /*Owner*/)
	: WorldPtr(InWorld), ClassToSpawn(InClass)
{
	if (!WorldPtr.IsValid() || !ClassToSpawn)
	{
		Debug::LogWarning(TEXT("[Pool] Invalid world or class passed to TObjectPool (constructor)"), true, 5.f);
		return;
	}

	// Debug::Log(FString::Printf(TEXT("[Pool] Creating pool for %s (size=%d) World=%s"), *GetNameSafe(*ClassToSpawn), InInitialSize, *GetNameSafe(WorldPtr.Get())), true, 5.f);

	for (int32 i = 0; i < InInitialSize; ++i)
	{
		if (T* Spawned = SpawnNew())
		{
			Pool.Add(Spawned);
		}
	}
}

template <typename T>
T* TObjectPool<T>::Acquire()
{
	// Cull invalid
	for (int32 i = 0; i < Pool.Num(); )
	{
		TWeakObjectPtr<T>& Entry = Pool[i];
		if (!Entry.IsValid())
		{
			Pool.RemoveAtSwap(i);
			continue;
		}

		T* Obj = Entry.Get();
		if (!IsValid(Obj))
		{
			Pool.RemoveAtSwap(i);
			continue;
		}

		// Free slot is one that is currently deactivated (tick disabled)
		if (!Obj->IsActorTickEnabled())
		{
			Obj->SetActorHiddenInGame(false);
			Obj->SetActorEnableCollision(true);
			Obj->SetActorTickEnabled(true);
			return Obj;
		}
		++i;
	}

	// Need a new instance
	T* NewObj = SpawnNew();
	if (IsValid(NewObj))
	{
		Pool.Add(NewObj);
		NewObj->SetActorHiddenInGame(false);
		NewObj->SetActorEnableCollision(true);
		NewObj->SetActorTickEnabled(true);
	}
	else
	{
		Debug::LogWarning(FString::Printf(TEXT("[Pool] SpawnNew failed for class %s (World valid=%d)"), *GetNameSafe(*ClassToSpawn), WorldPtr.IsValid() ? 1 : 0), true, 5.f);
	}
	return NewObj;
}

template <typename T>
void TObjectPool<T>::Release(T* Obj)
{
	if (!IsValid(Obj)) return;
	Obj->SetActorHiddenInGame(true);
	Obj->SetActorEnableCollision(false);
	Obj->SetActorTickEnabled(false);
}

template <typename T>
T* TObjectPool<T>::SpawnNew()
{
	UWorld* World = WorldPtr.Get();
	if (!World)
	{
		Debug::LogWarning(TEXT("[Pool] SpawnNew aborted: World is null (possibly map travel)"), true, 5.f);
		return nullptr;
	}
	if (!IsValid(ClassToSpawn))
	{
		Debug::LogWarning(TEXT("[Pool] SpawnNew aborted: ClassToSpawn invalid"), true, 5.f);
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	T* Obj = World->SpawnActor<T>(ClassToSpawn, FTransform::Identity, Params);

	if (!IsValid(Obj))
	{
		Debug::Log(TEXT("[Pool] SpawnActor returned invalid object"), true, 5.f);
		return nullptr;
	}

	// Put into inactive state immediately
	Release(Obj);
	return Obj;
}
