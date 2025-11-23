// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GP4Prototype/Public/Systems/FloorEventSystem/FloorEvent.h"
#include "FloorEventsManagerSubsystem.generated.h"


class ANavMeshBoundsVolume;

UCLASS(Blueprintable)
class GP4PROTOTYPE_API UFloorEventsManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseEventChance = 10.00f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentEventChance = BaseEventChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseEventChanceIncrease = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCurrentLevelEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Events")
	TArray<TSubclassOf<AFloorEvent>> AvailableEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Events")
	ANavMeshBoundsVolume* NavMeshVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Events")
	AFloorEvent* CurrentEvent;

	UFUNCTION(BlueprintCallable, Category = "Floor Events")
	bool IsEventSpawning();

	UFUNCTION(BlueprintCallable, Category = "Floor Events")
	void EventChanceIncrease(float ChanceIncrease);

	UFUNCTION(BlueprintCallable, Category = "Floor Events")
	void PickEvent(ANavMeshBoundsVolume* ANavMeshVolume);

	UFUNCTION(BlueprintCallable, Category = "Floor Events")
	void SpawnEvent(TSubclassOf<AFloorEvent> EventClass);

	UFUNCTION(BlueprintCallable, Category = "Floor Events")
	void IncreaseUpgradeChance();


protected: 

private:
	//UFloorEventsManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	int GetRandomWeightedIndex(TArray<int32> weights);

};
