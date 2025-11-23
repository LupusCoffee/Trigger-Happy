// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/AISpawningSystem/AISpawnSingle.h"
#include "NavigationSystem.h" 
#include "NavigationPath.h"

// Sets default values
AAISpawnSingle::AAISpawnSingle()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAISpawnSingle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAISpawnSingle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAISpawnSingle::StartSpawn()
{
	Super::StartSpawn();
	UE_LOG(LogTemp, Log, TEXT("Spawn point start spawning."));
	if (DelayBeforeSpawnStart > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			SpawnDelayTimerHandle,
			this,
			&AAISpawnSingle::SpawnSingleEnemy,
			DelayBeforeSpawnStart,
			false //NOT looping
		);
	}
	else
	{
		// No delay, spawn immediately
		SpawnSingleEnemy();
	}

}

void AAISpawnSingle::SpawnSingleEnemy()
{
	if (DelayBetweenSpawn <= 0.f || EnemiesToSpawn.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnSingleEnemy aborted: invalid delay or empty class list."));
		return;
	}

	float TimerToUse = 0.01f;
	if (!bIsHordeSpawner){TimerToUse = DelayBetweenSpawn;}
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AAISpawnSingle::HandleSpawn,
		TimerToUse,
		true   // Loop
	);

	HandleSpawn();
}
void AAISpawnSingle::HandleSpawn()
{
	if (EnemiesToSpawn.Num() == 0) return;
	
	if (AssignedSpawnerTrigger->EnemiesThatHaveSpawned < AssignedSpawnerTrigger->TotalToSpawnInRegion)
	{
		if (CapSpawnAmount == 0 || EnemiesThisSpawnerSpawned < CapSpawnAmount)
		{
			// Pick a random class
			int32 Index = FMath::RandRange(0, EnemiesToSpawn.Num() - 1);
			TSubclassOf<AAICharacterBase> ChosenEnemy = EnemiesToSpawn[Index];

			if (ChosenEnemy)
			{
				UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
				if (NavSystem)
				{
					FNavLocation ValidSpawnLocation;
					FVector Origin = GetActorLocation();

					// Pick a random point inside the box defined by SpawnSearchRadius
					FVector RandomOffset(
						FMath::FRandRange(-SpawnSearchRadius.X, SpawnSearchRadius.X),
						FMath::FRandRange(-SpawnSearchRadius.Y, SpawnSearchRadius.Y),
						FMath::FRandRange(-SpawnSearchRadius.Z, SpawnSearchRadius.Z)
					);

					FVector TestPoint = Origin + RandomOffset;

					// Project the random point onto the NavMesh to make sure it’s valid
					bool bFoundLocation = NavSystem->ProjectPointToNavigation(
						TestPoint,
						ValidSpawnLocation,
						SpawnSearchRadius // Use the extents as the query extent
					);

					if (bFoundLocation)
					{
						FVector SpawnLocation = ValidSpawnLocation.Location;
						FVector SpawnOffsetTotal = FVector(0.f, 0.f, SpawnOffset);   

						FVector FinalSpawnLocation = ValidSpawnLocation.Location + SpawnOffsetTotal;
						// Start above the NavMesh point and trace downward
						FVector TraceStart = SpawnLocation + FVector(0.f, 0.f, 500.f); // 500 units above
						FVector TraceEnd   = SpawnLocation - FVector(0.f, 0.f, 1000.f); // Trace down 1000 units

						FHitResult HitResult;
						FCollisionQueryParams QueryParams;
						QueryParams.bTraceComplex = true;
						QueryParams.AddIgnoredActor(this); // Ignore the spawner

						// Perform the line trace to detect the floor
						bool bHit = GetWorld()->LineTraceSingleByChannel(
							HitResult,
							TraceStart,
							TraceEnd,
							ECC_WorldStatic, // Trace against static world geometry (floor, walls, etc.)
							QueryParams
						);

						if (bHit)
						{
							// Use the surface impact point as the final spawn location
							SpawnLocation = HitResult.ImpactPoint;
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("No ground detected, using raw NavMesh location."));
						}
						FActorSpawnParameters SpawnParams;
						SpawnParams.Owner = this;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

						//Spawn enemy at the valid NavMesh point
						AAICharacterBase* SpawnedEnemy = GetWorld()->SpawnActor<AAICharacterBase>(
							ChosenEnemy,
							FinalSpawnLocation,
							GetActorRotation(), // Keep the same rotation as the spawner
							SpawnParams
						);

						if (SpawnedEnemy)
						{
							SpawnedEnemy->SpawnBrain = SpawnerBrain;
							SpawnerBrain->ActiveEnemies.AddUnique(SpawnedEnemy);
						}

						AssignedSpawnerTrigger->EnemiesThatHaveSpawned++;
						EnemiesThisSpawnerSpawned++;
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("No valid NavMesh point found for spawning!"));
					}
				}
			}
		}
		else
		{
			StopSpawn();
		}
	}
	else
	{
		StopSpawn();
	}
}

void AAISpawnSingle::StopSpawn()
{
	Super::StopSpawn();
	UE_LOG(LogTemp, Log, TEXT("Spawn point finished spawning."));
	bIsFinishedSpawning = true;
	OnSpawnFinished.Broadcast(this);
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}
