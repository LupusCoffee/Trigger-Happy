#include "Systems/CombatSystem/Bullets/Bullet.h"

#include "Components/CapsuleComponent.h"
#include "Core/Data/Enums/GameDamageType.h"
#include "Core/Data/Interfaces/Damageable.h"
#include "Systems/CombatSystem/CombatEventsSubsystem.h"
#include "Systems/CombatSystem/Components/WeakPoint.h"
#include "Systems/CombatSystem/Components/WeakPointComponent.h"
#include "ObjectPool/ObjectPoolSubsystem.h"


ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);

	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MoveComp"));
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();
	// Bind overlap once; pooled reuse won't duplicate bindings
	if (StaticMesh)
	{
		StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &ABullet::BulletHit);
	}
}

void ABullet::Init(UCombatEventsSubsystem* InCombatEventsSubsystem, float Damage, float MoveSpeed)
{
	CombatEventsSubsystem = InCombatEventsSubsystem;
	
	BaseDamage = Damage * BulletSpecificDamageMultiplier;
	BaseMoveSpeed = MoveSpeed * BulletSpecificSpeedMultiplier;

	MoveComp->MaxSpeed = MoveSpeed * BulletSpecificSpeedMultiplier;
}

void ABullet::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (StaticMesh)
	{
		StaticMesh->OnComponentBeginOverlap.RemoveDynamic(this, &ABullet::BulletHit);
	}
		
	Super::EndPlay(EndPlayReason);
}

void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveComp->Velocity = GetActorForwardVector() * BaseMoveSpeed;
}

void ABullet::BulletHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	if (OtherActor == this) return;
	if (OtherActor == GetInstigator()) return;

	FName HitSocketName = "";
	if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		if (USkeletalMeshComponent* Skeletal = Cast<USkeletalMeshComponent>(OtherComp))
		{
			if (IsValid(Skeletal)) HitSocketName = SweepResult.BoneName;
		}
		else if (UWeakPoint* WeakPointCapsule = Cast<UWeakPoint>(OtherComp))
		{
			HitSocketName = WeakPointCapsule->GetBoneAttachedTo();
		}

		IDamageable::Execute_TakeDamage(OtherActor, EGameDamageType::Gun, BaseDamage, 1.0f, HitSocketName);
	}

	OnHit.Broadcast(OtherActor, BaseDamage, HitSocketName, OverlappedComp, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	// Return to pool if available; otherwise destroy
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UObjectPoolSubsystem* Pool = GI->GetSubsystem<UObjectPoolSubsystem>())
		{
			Pool->ReleaseActor(this);
			// If no pool existed for this actor class, ReleaseActor is a no-op. Fallback to Destroy.
			if (IsActorTickEnabled())
			{
				Destroy();
			}
			return;
		}
	}

	Destroy();
}
