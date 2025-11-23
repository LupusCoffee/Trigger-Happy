// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/AISpawningSystem/AISpawnBrain.h"
#include "Character/AICharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/AISpawningSystem/AISpawnTrigger.h"


// Sets default values
AAISpawnBrain::AAISpawnBrain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ActorTypeToCollect = AAICharacterBase::StaticClass();
	TotalEnemiesForFloor = 0.0f;
}

// Called when the game starts or when spawned
void AAISpawnBrain::BeginPlay()
{
	Super::BeginPlay();
	CollectAllAICharacters();
	CollectTriggers();

}

// Called every frame
void AAISpawnBrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAISpawnBrain::CollectAllAICharacters()
{
	CollectedAICharacters.Empty();

	if (!ActorTypeToCollect) return;

	// Get all actors for the level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorTypeToCollect, FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AAICharacterBase* AICharacter = Cast<AAICharacterBase>(Actor))
		{
			CollectedAICharacters.Add(AICharacter);
			ActiveEnemies.AddUnique(AICharacter);
			AICharacter->SpawnBrain = this;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("SpawnerBrain collected %d AI characters."), CollectedAICharacters.Num());
}

void AAISpawnBrain::CollectPatrolRoutes()
{
	PatrolRoutes.Empty();

	TArray<AActor*> FoundRoutes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APatrolRoute::StaticClass(), FoundRoutes);

	for (AActor* Route : FoundRoutes)
	{
		if (APatrolRoute* Patrol = Cast<APatrolRoute>(Route))
		{
			PatrolRoutes.Add(Patrol);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SpawnerBrain collected %d patrol routes."), PatrolRoutes.Num());
}

void AAISpawnBrain::CollectTriggers()
{
	Triggers.Empty();

	TArray<AActor*> FoundTriggers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAISpawnTrigger::StaticClass(), FoundTriggers);

	for (AActor* Trigger : FoundTriggers)
	{
		if (AAISpawnTrigger* FoundTrigger = Cast<AAISpawnTrigger>(Trigger))
		{
			if (FoundTrigger->SpawnerBrain == this)
			{
				Triggers.Add(FoundTrigger);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SpawnerBrain collected %d triggers."), Triggers.Num());
	CalculateTotalAI();
}

void AAISpawnBrain::CalculateTotalAI()
{
	TotalEnemiesForFloor = 0;
	for (AAISpawnTrigger* Trigger : Triggers)
	{
		TotalEnemiesForFloor += Trigger->TotalToSpawnInRegion;
	}
	TotalEnemiesForFloor += CollectedAICharacters.Num();
}

void AAISpawnBrain::HandleAIDeath(AAICharacterBase* DeadAI)
{
	if (!DeadAI) return;
	if (ActiveEnemies.Contains(DeadAI))
	{
		ActiveEnemies.Remove(DeadAI);
		DefeatedEnemies.AddUnique(DeadAI);
	}

	// Clean up any stale/null entries before using ActiveEnemies to broadcast
	for (int32 Idx = ActiveEnemies.Num() - 1; Idx >= 0; --Idx)
	{
		if (!IsValid(ActiveEnemies[Idx]))
		{
			ActiveEnemies.RemoveAtSwap(Idx);
		}
	}
	// Avoid divide-by-zero and use float ratio for threshold comparison
	if (TotalEnemiesForFloor > 0)
	{
		const float RatioDefeated = static_cast<float>(DefeatedEnemies.Num()) / static_cast<float>(TotalEnemiesForFloor);
		if (!bIsLastManStanding && RatioDefeated >= LastManStandingAggroThreshold)
		{
			bIsLastManStanding = true;
			const TArray<AAICharacterBase*> Snapshot = ActiveEnemies;
			for (AAICharacterBase* ActiveAI : Snapshot)
			{
				if (IsValid(ActiveAI))
				{
					ActiveAI->LastManStanding.Broadcast();
				}
			}
		}
	}
}

void AAISpawnBrain::ApplyDifficultySpawnIncrease(int FloorNumber)
{
	if (Triggers.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No triggers collected in SpawnBrain."));
		return;
	}

	int BaseEnemiesPerTrigger = FloorNumber / Triggers.Num();
	int Remainder = FloorNumber % Triggers.Num();

	for (int i = 0; i < Triggers.Num(); i++)
	{
		if (Triggers[i])
		{
			int EnemiesForThisTrigger = BaseEnemiesPerTrigger;
			if (i < Remainder)
			{
				EnemiesForThisTrigger += 1;
			}

			Triggers[i]->TotalToSpawnInRegion += EnemiesForThisTrigger;
			UE_LOG(LogTemp, Log, TEXT("Trigger %d assigned %d enemies"), i, EnemiesForThisTrigger);
		}
	}
}
