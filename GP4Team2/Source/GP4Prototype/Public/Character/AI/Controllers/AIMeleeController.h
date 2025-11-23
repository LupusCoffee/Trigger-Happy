// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Character/AICharacterBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "AIMeleeController.generated.h"

UCLASS()
class GP4PROTOTYPE_API AAIMeleeController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAIMeleeController();

	// Perception Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerceptionComponent;

	// Sense Configurations
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Hearing* HearingConfig;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	AAICharacterBase* OwnedPossessedPawn;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnPossess(APawn* PossessedPawn);
	
};
