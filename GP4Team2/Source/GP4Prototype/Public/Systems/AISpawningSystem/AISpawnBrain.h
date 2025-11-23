// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AICharacterBase.h"
#include "GameFramework/Actor.h"
#include "AISpawnBrain.generated.h"

class AAISpawnTrigger;

UCLASS()
class GP4PROTOTYPE_API AAISpawnBrain : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAISpawnBrain();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	float TotalEnemiesForFloor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ToolTip = "The percent of defeated enemies before Last Man Standing event is called."))
	float LastManStandingAggroThreshold = 0.9;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bIsLastManStanding = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner")
	TArray<AAICharacterBase*> CollectedAICharacters;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<AAICharacterBase*> ActiveEnemies;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<AAICharacterBase*> DefeatedEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawner")
	TSubclassOf<AAICharacterBase> ActorTypeToCollect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner")
	TArray<APatrolRoute*> PatrolRoutes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner")
	TArray<AAISpawnTrigger*> Triggers;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void CollectAllAICharacters();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void CollectPatrolRoutes();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void CollectTriggers();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void CalculateTotalAI();

	UFUNCTION(BlueprintCallable, Category="Final Aggro")
	void HandleAIDeath(AAICharacterBase* DeadAI);

	UFUNCTION(BlueprintCallable, Category="Difficulty")
	void ApplyDifficultySpawnIncrease(int FloorNumber);
};
