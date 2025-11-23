// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/AISpawningSystem/AISpawnPoint.h"

#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"


// Sets default values
AAISpawnPoint::AAISpawnPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AAISpawnPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SelectedPatrolRoute = nullptr;

	if (SpawnerBrain)
	{
		for (APatrolRoute* Route : SpawnerBrain->PatrolRoutes)
		{
			if (!Route) continue;

#if WITH_EDITOR
			const FString Label = Route->GetActorLabel();
#else
			const FString Label = Route->GetName();
#endif
			if (Label == SelectedPatrolRouteName.ToString())
			{
				SelectedPatrolRoute = Route;
				break;
			}
		}
	}
	#if WITH_EDITOR
    FlushPersistentDebugLines(GetWorld()); // clear previous draws

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return;

    const float Step = 100.f;
    FVector Origin = GetActorLocation();

	for (float x = -SpawnSearchRadius.X; x <= SpawnSearchRadius.X; x += Step)
	{
		for (float y = -SpawnSearchRadius.Y; y <= SpawnSearchRadius.Y; y += Step)
		{
			// You can also add a Z loop if you want a 3D volume
			FVector TestPoint = Origin + FVector(x, y, 0.f);

			FNavLocation NavLoc;
			// Project the point onto the navmesh
			if (NavSys->ProjectPointToNavigation(
					TestPoint,
					NavLoc,
					FVector(50.f, 50.f, SpawnSearchRadius.Z) // search extent (Z controls how high/low we project)
				))
			{
				DrawDebugBox(
					GetWorld(),
					NavLoc.Location,
					FVector(10.f),       // half size of the debug cube
					FColor::Cyan,
					true                 // persistent until next FlushPersistentDebugLines
				);
			}
		}
	}
#endif
}

// Called when the game starts or when spawned
void AAISpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAISpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TArray<FName> AAISpawnPoint::GetAvailablePatrolRoutes() const
{
	TArray<FName> Names;

	if (SpawnerBrain)
	{
		for (APatrolRoute* Route : SpawnerBrain->PatrolRoutes)
		{
			if (!Route) continue;

#if WITH_EDITOR
			// Use the human-readable World Outliner name
			Names.Add(FName(*Route->GetActorLabel()));
#else
			// Fallback if built without editor
			Names.Add(Route->GetFName());
#endif
		}
	}
	return Names; 
}

void AAISpawnPoint::StartSpawn()
{
}

void AAISpawnPoint::StopSpawn()
{
}

