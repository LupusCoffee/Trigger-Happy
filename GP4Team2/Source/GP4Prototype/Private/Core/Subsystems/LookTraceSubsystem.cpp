#include "Core/Subsystems/LookTraceSubsystem.h"
#include "Core/Data/DeveloperSettings/LookTraceSettings.h"

void ULookTraceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LookTraceSettings = GetMutableDefault<ULookTraceSettings>();
}

FHitResult ULookTraceSubsystem::GetHitResultFromCameraSphereTrace(AController* Controller, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel)
{
	FHitResult Hit;

	if (!LookTraceSettings) return Hit;

	//Start and End Locations
	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
	FVector StartLocation = ViewLocation;
	FVector EndLocation = ViewLocation + ViewRotation.Vector() * TraceLength;
	
	//Sphere, Params
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SphereTraceSingle),false);
	Params.AddIgnoredActor(Controller->GetPawn()->GetOwner());

	//Shoot sphere trace
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, StartLocation, EndLocation,
		FQuat::Identity, TraceChannel, Sphere, Params
	);

	//Debug
	if (LookTraceSettings->DoesDrawDebug())
	{
		DrawDebugCapsule(
			GetWorld(), (StartLocation+EndLocation)*0.5f,
			(EndLocation-StartLocation).Size()*0.5f,TraceRadius,
			FRotationMatrix::MakeFromZ(ViewRotation.Vector()).ToQuat(),
			bHit ? FColor::Green : FColor::Red, false, 0.05f
		);
	}
	
	return Hit;
}

FHitResult ULookTraceSubsystem::GetHitResultFromPawnForwardSphereTrace(APawn* Pawn, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel)
{
	FHitResult Hit;

	if (!LookTraceSettings) return Hit;
	if (!Pawn) return Hit;

	//Start and End Locations
	FVector Forward = Pawn->GetActorForwardVector();
	FVector ResultTraceDir = Forward.GetSafeNormal();
	
	FVector StartLocation = Pawn->GetActorLocation();
	FVector EndLocation = Pawn->GetActorLocation() + (ResultTraceDir * TraceLength);
	
	//Sphere, Params
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SphereTraceSingle),false);

	//Shoot sphere trace
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, StartLocation, EndLocation,
		FQuat::Identity, TraceChannel, Sphere, Params
	);

	//Debug
	if (LookTraceSettings->DoesDrawDebug())
	{
		DrawDebugCapsule(
			GetWorld(), (StartLocation+EndLocation)*0.5f,
			(EndLocation-StartLocation).Size()*0.5f,TraceRadius,
			FRotationMatrix::MakeFromZ(ResultTraceDir).ToQuat(),
			bHit ? FColor::Green : FColor::Red, false, 0.05f
		);
	}

	return Hit;
}

FHitResult ULookTraceSubsystem::GetHitResultFromPawnForwardSphereTraceWithOffset(float ForwardOffset, APawn* Pawn,
	float TraceLength, float TraceRadius, ECollisionChannel TraceChannel)
{
	FHitResult Hit;

	if (!LookTraceSettings) return Hit;
	if (!Pawn) return Hit;

	//Start and End Locations
	FVector Forward = Pawn->GetActorForwardVector();
	FVector ResultTraceDir = Forward.GetSafeNormal();
	
	FVector StartLocation = Pawn->GetActorLocation() + Forward*ForwardOffset;
	FVector EndLocation = Pawn->GetActorLocation() + (ResultTraceDir * TraceLength);
	
	//Sphere, Params
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SphereTraceSingle),false);

	//Shoot sphere trace
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, StartLocation, EndLocation,
		FQuat::Identity, TraceChannel, Sphere, Params
	);

	//Debug
	if (LookTraceSettings->DoesDrawDebug())
	{
		DrawDebugCapsule(
			GetWorld(), (StartLocation+EndLocation)*0.5f,
			(EndLocation-StartLocation).Size()*0.5f,TraceRadius,
			FRotationMatrix::MakeFromZ(ResultTraceDir).ToQuat(),
			bHit ? FColor::Green : FColor::Red, false, 0.05f
		);
	}

	return Hit;
}

FHitResult ULookTraceSubsystem::GetHitResultFromPawnDownSphereTrace(APawn* Pawn, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel)
{
	FHitResult Hit;

	if (!LookTraceSettings) return Hit;
	if (!Pawn) return Hit;

	//Start and End Locations
	FVector Down = FVector(Pawn->GetActorUpVector().X, Pawn->GetActorUpVector().Y, Pawn->GetActorUpVector().Z * -1);
	FVector ResultTraceDir = Down.GetSafeNormal();
	
	FVector StartLocation = Pawn->GetActorLocation();
	FVector EndLocation = Pawn->GetActorLocation() + (ResultTraceDir * TraceLength);
	
	//Sphere, Params
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SphereTraceSingle),false);

	//Shoot sphere trace
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, StartLocation, EndLocation,
		FQuat::Identity, TraceChannel, Sphere, Params
	);

	//Debug
	if (LookTraceSettings->DoesDrawDebug())
	{
		DrawDebugCapsule(
			GetWorld(), (StartLocation+EndLocation)*0.5f,
			(EndLocation-StartLocation).Size()*0.5f,TraceRadius,
			FRotationMatrix::MakeFromZ(ResultTraceDir).ToQuat(),
			bHit ? FColor::Green : FColor::Red, false, 0.05f
		);
	}

	return Hit;
}

void ULookTraceSubsystem::GetHitResultFromCapsuleSweep(UPrimitiveComponent* BatComp, FVector& CurrentBase,
	FVector& CurrentTip, FVector& PrevBase, FVector& PrevTip, float Radius, TArray<FHitResult>& OutHits, ECollisionChannel TraceChannel)
{
	// setup
	OutHits.Reset();
    if (!BatComp) return;

    UWorld* World = BatComp->GetWorld();
    if (!World) return;

    FVector AxisVector = CurrentTip - CurrentBase;
    float Length = AxisVector.Size();
    if (Length <= 0.01f) return;

    FVector AxisDir = AxisVector / Length;
    float HalfHeight = Length * 0.5f;
    FVector PrevMid   = (PrevBase + PrevTip) * 0.5f;
    FVector CurrentMid= (CurrentBase + CurrentTip) * 0.5f;

	
    // align capsule along Z
    FQuat Rot = FRotationMatrix::MakeFromZ(AxisDir).ToQuat();

    FCollisionShape Capsule = FCollisionShape::MakeCapsule(Radius, HalfHeight);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(BatCapsuleSweep), false);
    Params.AddIgnoredActor(BatComp->GetOwner());
    Params.bReturnPhysicalMaterial = false;

    TArray<FHitResult> Hits;

	
    // main capsule sweep
    bool bHit = World->SweepMultiByChannel(
        Hits,
        PrevMid,
        CurrentMid,
        Rot,
        TraceChannel,
        Capsule,
        Params
    );

	
    // extra sweeps (cause dont wanna miss shit)
    auto SphereSweep = [&](const FVector& From, const FVector& To)
    {
        TArray<FHitResult> LocalHits;
        World->SweepMultiByChannel(
            LocalHits,
            From,
            To,
            FQuat::Identity,
            TraceChannel,
            FCollisionShape::MakeSphere(Radius),
            Params
        );
        Hits.Append(LocalHits);
    };

	
    // cover rotational arcs
    SphereSweep(PrevTip, CurrentTip);
    SphereSweep(PrevBase, CurrentBase);

	
	// debug
    if (LookTraceSettings->DoesDrawDebug())
    {
        DrawDebugCapsule(World, CurrentMid, HalfHeight, Radius, Rot,
            bHit ? FColor::Red : FColor::Green, false, 10.0f);
        DrawDebugLine(World, PrevMid, CurrentMid, FColor::Yellow, false, 0.05f, 0, 1.0f);
    	
        DrawDebugLine(World, PrevTip, CurrentTip, FColor::Cyan, false, 0.05f, 0, 1.0f);
        DrawDebugLine(World, PrevBase, CurrentBase, FColor::Blue, false, 0.05f, 0, 1.0f);
    }

	
    // combine
    if (Hits.Num() > 0)
        OutHits = MoveTemp(Hits);
}

FVector ULookTraceSubsystem::GetLocationFromCameraLineTrace(AController* Controller, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel) //gotta make this a line, not a sphere
{
	FHitResult Hit;

	if (!LookTraceSettings) return FVector::ZeroVector;

	//Start and End Locations
	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
	FVector StartLocation = ViewLocation;
	FVector EndLocation = ViewLocation + ViewRotation.Vector() * TraceLength;
	
	//Sphere, Params
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SphereTraceSingle),false);

	//Shoot sphere trace
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, StartLocation, EndLocation,
		FQuat::Identity, TraceChannel, Sphere, Params
	);

	//Debug
	if (LookTraceSettings->DoesDrawDebug())
	{
		DrawDebugCapsule(
			GetWorld(), (StartLocation+EndLocation)*0.5f,
			(EndLocation-StartLocation).Size()*0.5f,TraceRadius,
			FRotationMatrix::MakeFromZ(ViewRotation.Vector()).ToQuat(),
			bHit ? FColor::Green : FColor::Red, false, 0.05f
		);
	}

	return Hit.Location;
}

TArray<FHitResult> ULookTraceSubsystem::GetHitResultFromLocationWithDirectionSphereTrace(FVector Forward, FVector StartLocation, float TraceLength, float TraceRadius, ECollisionChannel TraceChannel)
{
	TArray<FHitResult> Hits;

	if (!LookTraceSettings) return Hits;

	//Start and End Locations
	FVector ResultTraceDir = Forward.GetSafeNormal();
	FVector EndLocation = StartLocation + (ResultTraceDir * TraceLength);
	
	//Sphere, Params
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SphereTraceSingle),false);

	//Shoot sphere trace
	const bool bHit = GetWorld()->SweepMultiByChannel(
		Hits, StartLocation, EndLocation,
		FQuat::Identity, TraceChannel, Sphere, Params
	);

	//Debug
	if (LookTraceSettings->DoesDrawDebug())
	{
		DrawDebugCapsule(
			GetWorld(), (StartLocation+EndLocation)*0.5f,
			(EndLocation-StartLocation).Size()*0.5f,TraceRadius,
			FRotationMatrix::MakeFromZ(ResultTraceDir).ToQuat(),
			bHit ? FColor::Green : FColor::Red, false, 5.0f
		);
	}

	return Hits;
}