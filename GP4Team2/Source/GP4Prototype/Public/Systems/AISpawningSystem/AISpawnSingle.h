// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AISpawnPoint.h"
#include "AISpawnSingle.generated.h"

UCLASS()
class GP4PROTOTYPE_API AAISpawnSingle : public AAISpawnPoint
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAISpawnSingle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Single")
	float DelayBeforeSpawnStart = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Single")
	bool bIsHordeSpawner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Single")
	float CapSpawnAmount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Single")
	float DelayBetweenSpawn = 1.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn|Single")
	float SpawnOffset = 120.f;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FTimerHandle SpawnTimerHandle;
	FTimerHandle SpawnDelayTimerHandle;
	float EnemiesThisSpawnerSpawned;
	void HandleSpawn();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void StartSpawn() override;
	virtual void StopSpawn() override;

	UFUNCTION(BlueprintCallable, Category="Spawn|Single")
	void SpawnSingleEnemy();
};
