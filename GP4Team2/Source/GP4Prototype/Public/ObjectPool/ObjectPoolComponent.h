// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolBase.h"
#include "Components/ActorComponent.h"
#include "ObjectPoolComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GP4PROTOTYPE_API UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void InitializePool(TSubclassOf<AActor> InClass, int32 InSize);

	UFUNCTION(BlueprintCallable)
	AActor* AcquireActor() const;

	UFUNCTION(BlueprintCallable)
	void ReleaseActor(AActor* Actor) const;

private:
	UPROPERTY()
	TObjectPtr<UObjectPoolBase> Pool;
};