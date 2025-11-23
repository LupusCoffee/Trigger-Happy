// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ObjectPoolComponent.h"

#include "ObjectPool/ObjectPoolBase.h"


void UObjectPoolComponent::InitializePool(TSubclassOf<AActor> InClass, int32 InSize)
{
	Pool = NewObject<UObjectPoolBase>(this);
	Pool->InitializePool(InClass, InSize);
}

AActor* UObjectPoolComponent::AcquireActor() const
{
	return Pool ? Pool->AcquireActor() : nullptr;
}

void UObjectPoolComponent::ReleaseActor(AActor* Actor) const
{
	if (Pool)
		Pool->ReleaseActor(Actor);
}
