// Fill out your copyright notice in the Description page of Project Settings.


#include "GP4Prototype/Public/Systems/FloorEventSystem/EventFloorCollectible.h"

// Sets default values
AEventFloorCollectible::AEventFloorCollectible()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEventFloorCollectible::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEventFloorCollectible::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEventFloorCollectible::OnFloorEventItemCollected_Implementation() {
	// more logic can be implemented in the individual blueprint
	
}