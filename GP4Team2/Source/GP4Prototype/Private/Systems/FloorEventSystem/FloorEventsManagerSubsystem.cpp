// Fill out your copyright notice in the Description page of Project Settings.


#include "GP4Prototype/Public/Systems/FloorEventSystem/FloorEventsManagerSubsystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"
void UFloorEventsManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("FloorEventManagerSubsystem Initializing"))
}

void UFloorEventsManagerSubsystem::Deinitialize() {
	Super::Deinitialize();
}

bool UFloorEventsManagerSubsystem::IsEventSpawning() {
	bool isSpawning = false;
	float RandomValue = FMath::RandRange(0.0f, 100.0f);

	if (RandomValue <= CurrentEventChance) {
		isSpawning = true;

		CurrentEventChance = BaseEventChance;
		
	}
	else {
		EventChanceIncrease(BaseEventChanceIncrease);
	}

	return isSpawning;
}


void UFloorEventsManagerSubsystem::PickEvent(ANavMeshBoundsVolume* ANavMeshVolume) {
	NavMeshVolume = ANavMeshVolume;

	TArray<int32> weights;
	weights.Reserve(AvailableEvents.Num());
	for (TSubclassOf<AFloorEvent> EventClass : AvailableEvents)
	{
		if (EventClass)
		{
			AFloorEvent* DefaultEvent = EventClass->GetDefaultObject<AFloorEvent>();
			weights.Add(DefaultEvent->EventChanceWeight);
		}
	}

	int index = GetRandomWeightedIndex(weights);

	TSubclassOf<AFloorEvent> ChosenClass = AvailableEvents[index];
	SpawnEvent(ChosenClass);
}



void UFloorEventsManagerSubsystem::EventChanceIncrease(float ChanceIncrease) {
	CurrentEventChance += ChanceIncrease;
}

void UFloorEventsManagerSubsystem::SpawnEvent(TSubclassOf<AFloorEvent> EventClass) {

	if (!EventClass) return;

	TSubclassOf<AFloorEvent> ChosenClass = EventClass;

	if (ChosenClass && GetWorld())
	{
		FVector SpawnLocation = FVector(0.f, 0.f, 100.f);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		AFloorEvent* SpawnedEvent = GetWorld()->SpawnActor<AFloorEvent>(ChosenClass, SpawnLocation, SpawnRotation);
		SpawnedEvent->NavMeshVolume = NavMeshVolume;

		if (SpawnedEvent)
		{
			// SpawnEvent() doesnt do anything, event logic is in the event blueprints
			SpawnedEvent->SpawnEvent(); 
		}
		CurrentEvent = SpawnedEvent;
		
	}

}
void UFloorEventsManagerSubsystem::IncreaseUpgradeChance() {

}

int UFloorEventsManagerSubsystem::GetRandomWeightedIndex(TArray<int32> weights) {

	int totalWeight=0;

	for (int i = 0; i < weights.Num(); i++) {
		totalWeight += weights[i];
	}
	int RandomValue = FMath::RandRange(0, totalWeight);

	for (int i = 0; i < weights.Num(); i++) {
		RandomValue -= weights[i];

		if (RandomValue <= 0) {
			return i;
		}
	}


	return 0;
}
