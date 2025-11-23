// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/AISpawningSystem/AISpawnTrigger.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "Systems/AISpawningSystem/AISpawnPoint.h"
#include "Systems/AISpawningSystem/AISpawnSingle.h"


// Sets default values
AAISpawnTrigger::AAISpawnTrigger()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TriggerBoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBoxCollider"));
	SetRootComponent(TriggerBoxCollider);
}

// Called when the game starts or when spawned
void AAISpawnTrigger::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAISpawnTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAISpawnTrigger::GetLinkedSpawnPoints()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AAISpawnPoint> It(World); It; ++It)
	{
		AAISpawnPoint* SpawnPoint = *It;
		if (SpawnPoint && SpawnPoint->AssignedSpawnerTrigger == this)
		{
			LinkedSpawnPoints.AddUnique(SpawnPoint);
		}
	}
	
}

void AAISpawnTrigger::TriggerSpawning()
{
	for (AAISpawnPoint* SpawnPoint : LinkedSpawnPoints)
	{
		if (AAISpawnSingle* SingleSpawn = Cast<AAISpawnSingle>(SpawnPoint))
		{
			if (SingleSpawn->bIsHordeSpawner && !SingleSpawn->bIsFinishedSpawning)
			{
				// Listen for this spawn point finishing
				SingleSpawn->OnSpawnFinished.AddDynamic(this, &AAISpawnTrigger::HandleSpawnFinished);
				SingleSpawn->StartSpawn();
				return;
			}
		}
	}

	for (AAISpawnPoint* SpawnPoint : LinkedSpawnPoints)
	{
		if (SpawnPoint && !SpawnPoint->bIsFinishedSpawning)
		{
			//SpawnPoint->OnSpawnFinished.AddDynamic(this, &AAISpawnTrigger::HandleSpawnFinished);
			SpawnPoint->StartSpawn();
			//return;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("All spawn points have finished spawning."));
}

void AAISpawnTrigger::HandleSpawnFinished(AAISpawnPoint* FinishedSpawner)
{
	//if (!FinishedSpawner) return;
	UE_LOG(LogTemp, Log, TEXT("One Spawn point finished spawning, call for next."));
	FinishedSpawner->OnSpawnFinished.RemoveDynamic(this, &AAISpawnTrigger::HandleSpawnFinished);
	TriggerSpawning();
}

