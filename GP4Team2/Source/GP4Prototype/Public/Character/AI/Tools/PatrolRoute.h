// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "PatrolRoute.generated.h"

UCLASS()
class GP4PROTOTYPE_API APatrolRoute : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APatrolRoute();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	USplineComponent* PatrolSpline;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
