// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EventFloorCollectible.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollectedDispatcher);

UCLASS()
class GP4PROTOTYPE_API AEventFloorCollectible : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEventFloorCollectible();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Event collectible")
	FText CollectibleName;

	UPROPERTY(BlueprintAssignable,BlueprintCallable, Category = "Floor Event collectible")
	FOnCollectedDispatcher OnCollectedDispatcher;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Floor Event collectible")
	void OnFloorEventItemCollected();
};
