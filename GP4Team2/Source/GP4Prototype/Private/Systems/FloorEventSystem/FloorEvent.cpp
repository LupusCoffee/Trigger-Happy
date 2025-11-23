// Fill out your copyright notice in the Description page of Project Settings.


#include "GP4Prototype/Public/Systems/FloorEventSystem/FloorEvent.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"
// Sets default values
AFloorEvent::AFloorEvent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFloorEvent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFloorEvent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFloorEvent::SpawnEvent() {

	// GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Spawning event: %s"),*GetName() ));
}

void AFloorEvent::CompleteEvent_Implementation() {
	//Required implementation, Every Floor Event Blueprint has their own implementation

}

void AFloorEvent::FailEvent_Implementation() {
	//Required implementation, Every Floor Event Blueprint has their own implementation
}

//FVector AFloorEvent::GetRandomNavMeshPoint()
//{
//        if (!NavMeshVolume) return FVector::ZeroVector;
//
//        UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
//        if (!NavSys) return FVector::ZeroVector;
//
//        FBox Bounds = NavMeshVolume->GetComponentsBoundingBox();
//
//        FVector RandomLocation = FMath::RandPointInBox(Bounds);
//
//        FNavLocation ProjectedLocation;
//        if (NavSys->ProjectPointToNavigation(RandomLocation, ProjectedLocation))
//        {
//            return ProjectedLocation.Location;
//        }
//
//        return FVector::ZeroVector;
//}
