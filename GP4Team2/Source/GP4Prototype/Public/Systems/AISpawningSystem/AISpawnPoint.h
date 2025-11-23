// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AISpawnBrain.h"
#include "AISpawnTrigger.h"
#include "GameFramework/Actor.h"
#include "AISpawnPoint.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawnFinished, AAISpawnPoint*, SpawnPoint);

UCLASS()
class GP4PROTOTYPE_API AAISpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAISpawnPoint();

	UPROPERTY(EditAnywhere, Category="Spawn|Setup")
	AAISpawnBrain* SpawnerBrain;

	/** Patrol route selected from SpawnerBrain's list */
	UPROPERTY(EditAnywhere, Category="Spawn|Setup", meta=(GetOptions="GetAvailablePatrolRoutes"))
	FName SelectedPatrolRouteName;

	/** Actual patrol route reference */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawn|Setup")
	APatrolRoute* SelectedPatrolRoute;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawn|Setup")
	bool bIsFinishedSpawning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Setup")
	AAISpawnTrigger* AssignedSpawnerTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Setup")
	TArray<TSubclassOf<AAICharacterBase>> EnemiesToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Single")
	FVector SpawnSearchRadius = FVector(1000.0f, 1000.0f, 1000.f);

	UPROPERTY(BlueprintAssignable, Category="Spawn|Events")
	FOnSpawnFinished OnSpawnFinished;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(CallInEditor)
	TArray<FName> GetAvailablePatrolRoutes() const;

	UFUNCTION(BlueprintCallable, Category="Spawn")
	virtual void StartSpawn();
	
	UFUNCTION(BlueprintCallable, Category="Spawn")
	virtual void StopSpawn();
};
