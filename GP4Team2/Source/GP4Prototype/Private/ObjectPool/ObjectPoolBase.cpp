// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ObjectPoolBase.h"

void UObjectPoolBase::InitializePool(TSubclassOf<AActor> InClass, int32 InSize)
{
	PooledClass = InClass;
	InitialSizeCached = InSize;
	CachedWorld = GetWorld();
	if (PoolInstance)
	{
		delete PoolInstance;
		PoolInstance = nullptr;
	}
	PoolInstance = new TObjectPool<AActor>(CachedWorld.Get(), InClass, InSize, this);
	SyncGCProtection();
}

void UObjectPoolBase::RebuildPoolForWorldChange()
{
	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld == CachedWorld)
	{
		return; // no change
	}
	// Tear down old pool
	if (PoolInstance)
	{
		delete PoolInstance;
		PoolInstance = nullptr;
	}
	GCProtectedPool.Empty();
	CachedWorld = CurrentWorld;
	if (!CachedWorld.Get() || !PooledClass)
	{
		return; // cannot rebuild
	}
	PoolInstance = new TObjectPool<AActor>(CachedWorld.Get(), PooledClass, InitialSizeCached, this);
	SyncGCProtection();
}

AActor* UObjectPoolBase::AcquireActor()
{
	// Detect world change (PIE restart / map travel) and rebuild if needed
	if (GetWorld() != CachedWorld)
	{
		RebuildPoolForWorldChange();
	}
	// Refresh weak world pointer inside pool
	if (PoolInstance)
	{
		PoolInstance->RefreshWorld(GetWorld());
	}
	AActor* Actor = PoolInstance ? PoolInstance->Acquire() : nullptr;
	if (Actor)
	{
		SyncGCProtection();
		return Actor;
	}

	// Pool exhausted: auto-grow by doubling (add InitialSizeCached more)
	const int32 Additional = FMath::Max(1, InitialSizeCached);
	if (GrowPool(Additional) && PoolInstance)
	{
		Actor = PoolInstance->Acquire();
		if (Actor)
		{
			SyncGCProtection();
			return Actor;
		}
	}

	return nullptr;
}

bool UObjectPoolBase::GrowPool(int32 AdditionalCount)
{
	if (!PoolInstance || !PooledClass || !CachedWorld.IsValid() || AdditionalCount <= 0)
	{
		return false;
	}

	UWorld* World = CachedWorld.Get();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 Spawned = 0;
	for (int32 i = 0; i < AdditionalCount; ++i)
	{
		if (!World) break;
		AActor* NewActor = World->SpawnActor<AActor>(PooledClass, FTransform::Identity, Params);
		if (NewActor)
		{
			// Return the new actor into the pool's free list
			PoolInstance->Release(NewActor);
			++Spawned;
		}
	}

	if (Spawned > 0)
	{
		InitialSizeCached += Spawned;
		SyncGCProtection();
		return true;
	}

	return false;
}

void UObjectPoolBase::ReleaseActor(AActor* Actor)
{
	if (PoolInstance && Actor)
	{
		PoolInstance->Release(Actor);
	}
}

void UObjectPoolBase::SyncGCProtection()
{
	if (!PoolInstance) return;
	GCProtectedPool.Empty();
	for (const TWeakObjectPtr<AActor>& WeakActor : PoolInstance->GetPool())
	{
		if (WeakActor.IsValid())
		{
			GCProtectedPool.Add(WeakActor.Get());
		}
	}
}

void UObjectPoolBase::BeginDestroy()
{
	Super::BeginDestroy();
	if (PoolInstance)
	{
		delete PoolInstance;
		PoolInstance = nullptr;
	}
}