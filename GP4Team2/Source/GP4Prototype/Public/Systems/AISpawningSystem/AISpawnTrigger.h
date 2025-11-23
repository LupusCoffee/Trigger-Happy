// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AISpawnBrain.h"
#include "GameFramework/Actor.h"
#include "AISpawnTrigger.generated.h"

class AAISpawnPoint;
class UBoxComponent;

UCLASS()
class GP4PROTOTYPE_API AAISpawnTrigger : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAISpawnTrigger();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	AAISpawnBrain* SpawnerBrain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	UBoxComponent* TriggerBoxCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawn")
	TArray<AAISpawnPoint*> LinkedSpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	float TotalToSpawnInRegion;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawn")
	float EnemiesThatHaveSpawned;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Spawn")
	void GetLinkedSpawnPoints();

	UFUNCTION(BlueprintCallable, Category="Spawn")
	void TriggerSpawning();

	UFUNCTION(BlueprintCallable, Category="Spawn")
	void HandleSpawnFinished(AAISpawnPoint* FinishedSpawner);
};
