// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorEvent.generated.h"


UENUM(BlueprintType, Category = "Floor Events")
enum EventState {
	Completed,
	Failed,
	Ongoing
};

class ANavMeshBoundsVolume;

UCLASS(Blueprintable)
class GP4PROTOTYPE_API AFloorEvent : public AActor
{
	GENERATED_BODY()
	
public:	
	AFloorEvent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Events")
	int EventChanceWeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Events")
	ANavMeshBoundsVolume* NavMeshVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Events")
	bool bIsEventOver = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Events")
	FText EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Events")
	FText EventDescription;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Floor Events")
	void SpawnEvent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Floor Events")
	void CompleteEvent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Floor Events")
	void FailEvent();
	
	/*UFUNCTION(BlueprintCallable, Category = "Floor Events")
	FVector GetRandomNavMeshPoint();*/

};
