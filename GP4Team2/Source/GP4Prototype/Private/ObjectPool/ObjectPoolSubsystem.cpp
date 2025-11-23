// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ObjectPoolSubsystem.h"
#include "Debug.h"

UObjectPoolBase* UObjectPoolSubsystem::GetOrCreatePool(TSubclassOf<AActor> ActorClass, int32 DefaultSize)
{
	if (!ActorClass)
	{
		Debug::Log(TEXT("[PoolSubsystem] GetOrCreatePool called with null ActorClass"), bDebugOnScreen, DebugDuration);
		return nullptr;
	}
	
	if (UObjectPoolBase* ExistingPool = Pools.FindRef(ActorClass))
	{
		const FString ClassName = GetNameSafe(*ActorClass);
		Debug::Log(FString::Printf(TEXT("[PoolSubsystem] Found existing pool for %s: %p"), *ClassName, ExistingPool), bDebugOnScreen, DebugDuration);
		return ExistingPool;
	}

	// Create new pool and add it to the map BEFORE initialization to handle callbacks during init
	const FString ClassName = GetNameSafe(*ActorClass);
	Debug::Log(FString::Printf(TEXT("[PoolSubsystem] Creating new pool for %s with size %d"), *ClassName, DefaultSize), bDebugOnScreen, DebugDuration);

	UObjectPoolBase* NewPool = NewObject<UObjectPoolBase>(this);
	Pools.Add(ActorClass, NewPool); // pre-register to avoid "No pool found" during init-time callbacks
	NewPool->InitializePool(ActorClass, DefaultSize);

	return NewPool;
}

void UObjectPoolSubsystem::RegisterPool(TSubclassOf<AActor> ActorClass, int32 InitialSize)
{
	const FString ClassName = GetNameSafe(*ActorClass);
	Debug::Log(FString::Printf(TEXT("[PoolSubsystem] RegisterPool %s size=%d"), *ClassName, InitialSize), bDebugOnScreen, DebugDuration);
	GetOrCreatePool(ActorClass, InitialSize);
}

AActor* UObjectPoolSubsystem::AcquireActor(TSubclassOf<AActor> ActorClass)
{
	const FString ClassName = GetNameSafe(*ActorClass);
	Debug::Log(FString::Printf(TEXT("[PoolSubsystem] AcquireActor %s"), *ClassName), bDebugOnScreen, DebugDuration);
	if (UObjectPoolBase* Pool = GetOrCreatePool(ActorClass))
	{
		AActor* Actor = Pool->AcquireActor();
		Debug::Log(FString::Printf(TEXT("[PoolSubsystem] AcquireActor -> %p"), Actor), bDebugOnScreen, DebugDuration);
		return Actor;
	}
	Debug::Log(TEXT("[PoolSubsystem] AcquireActor -> nullptr (no pool)"), bDebugOnScreen, DebugDuration);
	return nullptr;
}

void UObjectPoolSubsystem::ReleaseActor(AActor* Actor)
{
	if (!Actor)
	{
		Debug::Log(TEXT("[PoolSubsystem] ReleaseActor called with null Actor"), bDebugOnScreen, DebugDuration);
		return;
	}

	// Exact class match first
	TSubclassOf<AActor> ActorClass = Actor->GetClass();
	UObjectPoolBase* Pool = Pools.FindRef(ActorClass);

	// Fallback: scan for a pool whose pooled class matches or is related
	if (!Pool)
	{
		for (const auto& KVP : Pools)
		{
			if (!KVP.Value) continue;
			UClass* PooledClass = *KVP.Value->GetPooledClass();
			if (!PooledClass) continue;

			// Match exact or inheritance to be resilient against hot-reload/PIE edge cases
			if (ActorClass == PooledClass || ActorClass->IsChildOf(PooledClass) || PooledClass->IsChildOf(ActorClass))
			{
				Pool = KVP.Value;
				break;
			}
		}
	}

	const FString ActorName = GetNameSafe(Actor);
	const FString ActorClassName = GetNameSafe(ActorClass);

	if (Pool)
	{
		const FString PooledClassName = Pool->GetPooledClass() ? GetNameSafe(*Pool->GetPooledClass()) : FString(TEXT("<null>"));
		Debug::Log(FString::Printf(TEXT("[PoolSubsystem] Releasing %s (%p) to pool %p (PooledClass=%s)"),
			*ActorName, Actor, Pool, *PooledClassName), bDebugOnScreen, DebugDuration);
		Pool->ReleaseActor(Actor);
	}
	else
	{
		Debug::Log(FString::Printf(TEXT("[PoolSubsystem] No pool found for actor %s (%p) Class=%s; ignoring"),
			*ActorName, Actor, *ActorClassName), bDebugOnScreen, DebugDuration);
	}
}