#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "LookTraceSubsystem.generated.h"


class ULookTraceSettings;

UCLASS()
class GP4PROTOTYPE_API ULookTraceSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	//Methods
	UFUNCTION()
	FHitResult GetHitResultFromCameraSphereTrace(AController* Controller, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel);

	UFUNCTION()
	FHitResult GetHitResultFromPawnForwardSphereTrace(APawn* Pawn, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel);

	UFUNCTION()
	FHitResult GetHitResultFromPawnForwardSphereTraceWithOffset(float ForwardOffset, APawn* Pawn, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel);

	UFUNCTION()
	FHitResult GetHitResultFromPawnDownSphereTrace(APawn* Pawn, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel);

	UFUNCTION()
	void GetHitResultFromCapsuleSweep(UPrimitiveComponent* BatComp, FVector& CurrentBase, FVector& CurrentTip, FVector& PrevBase,
											FVector& PrevTip, float Radius, TArray<FHitResult>& OutHits, ECollisionChannel TraceChannel);
	
	UFUNCTION()
	FVector GetLocationFromCameraLineTrace(AController* Controller, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel);

	// from location and with direction
	UFUNCTION()
	TArray<FHitResult> GetHitResultFromLocationWithDirectionSphereTrace(FVector Forward, FVector StartLocation, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	//Variables
	UPROPERTY()
	ULookTraceSettings* LookTraceSettings;
};
