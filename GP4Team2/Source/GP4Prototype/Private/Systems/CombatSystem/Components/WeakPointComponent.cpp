#include "Systems/CombatSystem/Components/WeakPointComponent.h"


UWeakPointComponent::UWeakPointComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWeakPointComponent::BeginPlay()
{
	Super::BeginPlay();

	// get skeletal mesh of owner
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	GetOwner()->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
	for (USkeletalMeshComponent* Skel : SkeletalMeshes)
	{
		if (!Skel || !Skel->ComponentHasTag(TagOfDamageableSkelMesh)) continue;
		SkeletalMeshComponent = Skel;
	}

	// add all hit capsules
	if (!SkeletalMeshComponent) return;
	for (auto WeakPoint : WeakPoints)
	{
		bool UseSpecialHitCapsule = WeakPoint.Value.UseSpecialHitCapsule;
		if (!UseSpecialHitCapsule) continue;
		
		FName WeakPointBoneName = WeakPoint.Key;
		float Radius = WeakPoint.Value.HitCapsuleRadius;
		float HalfHeight = WeakPoint.Value.HitCapsuleHalfHeight;
		FRotator Rotation = WeakPoint.Value.Rotation;

		CreateWeakPoint(SkeletalMeshComponent, WeakPointBoneName, Radius, HalfHeight, Rotation, true, CollisionPresetName);
	}
}

void UWeakPointComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!DrawDebug) return;
	for (auto Capsule : CreatedWeakPoints) DrawHitCapsuleDebug(Capsule, 0);
}

UWeakPoint* UWeakPointComponent::CreateWeakPoint(USkeletalMeshComponent* TargetMesh, FName BoneName, float Radius, float HalfHeight, FRotator Rotation, bool ShouldGenerateOverlapEvents, FName CapsuleCollisionPresetName)
{	
	if (!TargetMesh) return nullptr;

	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;
	
	UWeakPoint* WeakPoint = NewObject<UWeakPoint>(Owner, UWeakPoint::StaticClass(), *FString::Printf(TEXT("WeakPoint_%s"), *BoneName.ToString()));
	if (!WeakPoint) return nullptr;
	
	WeakPoint->SetCapsuleRadius(Radius);
	WeakPoint->SetCapsuleHalfHeight(HalfHeight);
	WeakPoint->SetGenerateOverlapEvents(ShouldGenerateOverlapEvents);
	WeakPoint->SetCollisionProfileName(CapsuleCollisionPresetName);
	WeakPoint->RegisterComponent(); // important!

	// Attach to a bone on the skeletal mesh
	WeakPoint->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneName);
	WeakPoint->SetRelativeRotation(Rotation);

	// Init and Store
	WeakPoint->Init(BoneName);
	CreatedWeakPoints.Add(WeakPoint);
	
	return WeakPoint;
}

void UWeakPointComponent::DrawHitCapsuleDebug(UCapsuleComponent* Capsule, float Duration)
{
	if (!GetWorld()) return;

	if (!Capsule) return;

	float Radius, HalfHeight;
	Capsule->GetScaledCapsuleSize(Radius, HalfHeight);

	const FVector Center = Capsule->GetComponentLocation();
	const FQuat   Rot    = Capsule->GetComponentQuat();

	DrawDebugCapsule(
		GetWorld(),
		Center,
		HalfHeight,
		Radius,
		Rot,
		FColor::Red,
		/*bPersistentLines*/ false,
		/*LifeTime*/ Duration,
		/*DepthPriority*/ 0,
		/*Thickness*/ 1.5f
	);
}
