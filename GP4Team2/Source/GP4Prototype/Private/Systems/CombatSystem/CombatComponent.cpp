#include "Systems/CombatSystem/CombatComponent.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "Systems/AttributeSystem/AttributeComponent.h"
#include "Systems/CombatSystem/States/MeleeState.h"
#include "Systems/CombatSystem/CombatFiniteStateMachine.h"
#include "Systems/CombatSystem/States/AbilityState.h"
#include "Systems/CombatSystem/States/FireState.h"
#include "Systems/CombatSystem/States/IdleState.h"
#include "ObjectPool/ObjectPoolSubsystem.h"
#include "Debug.h"


class UAttributeComponent;
//Core Overrides and Constructors
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	
	//Initial Actor and Pawn stuff
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn) return;

	if (Owner) AttributeComponent = Owner->GetComponentByClass<UAttributeComponent>();
	
	CombatEventsSubsystem = GetWorld()->GetSubsystem<UCombatEventsSubsystem>();

	// look trace subsystem
	PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!PlayerController) return;
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	LookTraceSubsystem = LocalPlayer->GetSubsystem<ULookTraceSubsystem>();

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		PoolSubsystem = GI->GetSubsystem<UObjectPoolSubsystem>();
		Debug::Log(FString::Printf(TEXT("[Combat] PoolSubsystem cached = %p"), PoolSubsystem), bDebugOnScreen, DebugDuration);
		if (PoolSubsystem)
		{
			// Pre-register pools
			if (StandardBullet)
			{
				const FString StdName = GetNameSafe(*StandardBullet);
				Debug::Log(FString::Printf(TEXT("[Combat] RegisterPool StandardBullet=%s size=%d"), *StdName, BulletPoolInitialSize), bDebugOnScreen, DebugDuration);
				PoolSubsystem->RegisterPool(StandardBullet, BulletPoolInitialSize);
			}
			if (OneShotBullet)
			{
				int32 OneShotSize = FMath::Max(1, BulletPoolInitialSize / 2);
				const FString OneShotName = GetNameSafe(*OneShotBullet);
				Debug::Log(FString::Printf(TEXT("[Combat] RegisterPool OneShotBullet=%s size=%d"), *OneShotName, OneShotSize), bDebugOnScreen, DebugDuration);
				PoolSubsystem->RegisterPool(OneShotBullet, OneShotSize);
			}
		}
	}

	//Set Starter Values
	CurrentFireChargeCapacity = MaxFireChargeCapacity;

	float MeleeCooldown = 0;
	if (AttributeComponent) MeleeCooldown = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Melee_Cooldown);
	else MeleeCooldown = FallbackMeleeCooldown;
	
	CurrentMeleeCooldownTime = MeleeCooldown;

	TimeSinceLastFire = TimeBeforeFireRechargeBeginAfterFire;

	InputContext.bFireChargeFullyDepleted = false;


	
	//Set meshes -------------------------------------------------------------------------------------
	// char arms
	CharacterArms = GetSkelMeshByTag(CharacterArmsTag);
	if (!CharacterArms) return;

	CharacterArmAnimInstance = CharacterArms->GetAnimInstance();

	
	// gun mesh
	for (auto Element : GetOwner()->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), "GunMeshComp"))
	{
		GunMeshComp = Cast<USkeletalMeshComponent>(Element);
	}
	
	GunMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMeshComp->SetVisibility(true);

	GetOwner()->AddInstanceComponent(GunMeshComp);
	GunMeshComp->RegisterComponent();
	
	GunMeshComp->AttachToComponent(
		CharacterArms,
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		CharArmsGunSocketName);

	GunMeshComp->SetAnimInstanceClass(GunAnimBlueprint);

	
	// bat mesh
	BatMeshComp = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("BatMeshComp"));
	BatMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BatMeshComp->SetVisibility(false);
	if (BatMeshAsset) BatMeshComp->SetStaticMesh(BatMeshAsset);

	GetOwner()->AddInstanceComponent(BatMeshComp);
	BatMeshComp->RegisterComponent();
	
	BatMeshComp->AttachToComponent(
		CharacterArms,
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		CharArmsBatSocketName);
	// -------------------------------------------------------------------------------------
	

	
	//Combat Finite State Machine ----------------------------------------------------------
	// fsm setup
	CombatFSM = NewObject<UCombatFiniteStateMachine>();
	CombatFSM->Init(Owner, Pawn);
	
	UIdleState* IdleState = NewObject<UIdleState>();
	UFireState* FireState = NewObject<UFireState>();
	UMeleeState* MeleeState = NewObject<UMeleeState>();
	UAbilityState* AbilityState = NewObject<UAbilityState>();
	
	CombatFSM->SetupStates({
		IdleState,
		FireState,
		MeleeState,
		AbilityState
	});
	
	FireState->Init(FallbackFireDamage, FallbackFireDelay, FireTraceRange, FireTraceRadius, CharacterArms, GunFireMontage);
	MeleeState->Init(FallbackMeleeDamage,FallbackForwardMeleeKnockbackStrengthForSurvivingEnemies,
					 UpwardMeleeKnockbackStrengthForSurvivingEnemies, FallbackForwardMeleeKnockbackStrengthForDyingEnemies,
					 MeleeForwardStartOffset, MeleeRadius, MeleeReach, MeleeWidth, CombatTraceChannel, FallbackMaxMeleeHitCapacity,
					 MeleeDuration, BatMeleeMontage,BatMeshComp, GunMeshComp,
					 BatBottomSocket, BatTipSocket, MeleeStartNotify, MeleeEndNotify);
	AbilityState->Init();

	// inital state set
	CombatFSM->SetState(UIdleState::StaticClass(), InputContext);
	// ---------------------------------------------------------------------------------------
}

USkeletalMeshComponent* UCombatComponent::GetSkelMeshByTag(FName Tag)
{
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	GetOwner()->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
	
	for (USkeletalMeshComponent* Skel : SkeletalMeshes)
	{
		if (!Skel || !Skel->ComponentHasTag(Tag)) continue;
		return Skel;
	}

	return nullptr;
}


//Tick
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickFireCooldown(DeltaTime);
	TickFireRecharge(DeltaTime);
	TickMeleeCooldown(DeltaTime);
	CombatFSM->Tick(DeltaTime, InputContext);
}


//Value Setting
void UCombatComponent::TickFireCooldown(float DeltaTime)
{
	float FireDelay = FallbackFireDelay;
	if (AttributeComponent) FireDelay = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Weapon_FireDelay);
	
	if (CurrentFireCooldownTime < FireDelay)
	{
		CurrentFireCooldownTime += DeltaTime;
		InputContext.bHasNoFireCooldown = false;
	}
	else InputContext.bHasNoFireCooldown = true;
}

void UCombatComponent::TickFireRecharge(float DeltaTime)
{
	FireRechargeContext.Broadcast(CurrentFireChargeCapacity, MaxFireChargeCapacity, InputContext.bFireChargeFullyDepleted, UnderThreshold);


	//so we get stuck at the top until we let go
	if (InputContext.bFireChargeFullyDepleted && !InputContext.bWantsToFire)
	{
		CurrentFireRechargeRate = FullyDepletedRechargeRate;
	}
	
	
	// if under threshold, set bool
	if (CurrentFireChargeCapacity < MaxFireChargeCapacity*RechargeThresholdAfterFullDepletion && !InputContext.bFireChargeFullyDepleted) UnderThreshold = true;
	else UnderThreshold = false;
	

	// return before actual recharge if time since last fire is less than threshold
	TimeSinceLastFire += DeltaTime;
	if (TimeSinceLastFire < TimeBeforeFireRechargeBeginAfterFire) return;


	// actual recharge
	CurrentFireChargeCapacity += DeltaTime * CurrentFireRechargeRate;
	

	// limit FireChargeCapacity to MaxFireChargeCapacity
	if (CurrentFireChargeCapacity > MaxFireChargeCapacity) CurrentFireChargeCapacity = MaxFireChargeCapacity;
	
	// reset full depletion state upon going past threshold
	if (CurrentFireChargeCapacity > MaxFireChargeCapacity*RechargeThresholdAfterFullDepletion)
	{
		InputContext.bFireChargeFullyDepleted = false;
	}
}

void UCombatComponent::TickMeleeCooldown(float DeltaTime)
{
	float MeleeCooldown = FallbackMeleeCooldown;
	if (AttributeComponent) MeleeCooldown = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Melee_Cooldown);
	
	if (CurrentMeleeCooldownTime < MeleeCooldown)
	{
		CurrentMeleeCooldownTime += DeltaTime;
		InputContext.bCanMelee = false;
	}
	else InputContext.bCanMelee = true;
}

void UCombatComponent::OnFireExecuted()
{
	//Fire
	CurrentFireCooldownTime = 0;
	

	//Gun Recharge
	// zero time since last shot
	TimeSinceLastFire = 0;

	// consume charge
	CurrentFireChargeCapacity -= FireChargeConsumption;
	

	// set fully depleted to true if within threshold + zero CurrentFireChargeCapacity
	float FullDepletionThreshold = MaxFireChargeCapacity*RechargeThresholdAfterFullDepletion;
	if (CurrentFireChargeCapacity <= 0 || (InputContext.bFireChargeFullyDepleted && CurrentFireChargeCapacity < FullDepletionThreshold))
	{
		CurrentFireChargeCapacity = 0;
		
		InputContext.bFireChargeFullyDepleted = true;
	}

	// set recharge rate based on depletion state
	if (InputContext.bFireChargeFullyDepleted)
	{
		CurrentFireRechargeRate = 0;
	}
	else
	{
		float RechargeRateRegular = FallbackRegularRechargeRate;
		if (AttributeComponent) RechargeRateRegular = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Weapon_ReloadSpeed);
		CurrentFireRechargeRate = RechargeRateRegular;
	}
}

void UCombatComponent::OnMeleeStarted()
{
	InputContext.bWantsToMelee = false;
}

void UCombatComponent::OnMeleeFinished()
{
	CurrentMeleeCooldownTime = 0;
}

void UCombatComponent::OnAbilityStarted()
{
	InputContext.bWantsToAbility = false;
}


//Verbs (?)
void UCombatComponent::Fire()
{
	if (!GetWorld()) return;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = GetOwner()->GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// direction calculation
	FTransform FirePoint = GunMeshComp->GetSocketTransform(GunMeshFirePointSocket);

	if (!PlayerController) return;
	FVector AimPoint = LookTraceSubsystem->GetLocationFromCameraLineTrace(PlayerController, 100000.0f, 1.0f, ECollisionChannel::ECC_Visibility);

	FRotator ShootRotation;
	if (AimPoint == FVector(0,0,0)) ShootRotation = FirePoint.Rotator();
	else ShootRotation = (AimPoint - FirePoint.GetLocation()).GetSafeNormal().Rotation();
	
	// resolve bullet class
	TSubclassOf<ABullet> BulletToFire = UseOneShotBullet ? OneShotBullet : StandardBullet;
	const FString BulletClassName = BulletToFire ? GetNameSafe(*BulletToFire) : FString(TEXT("<null>"));
	Debug::Log(FString::Printf(TEXT("[Combat] Fire: UseOneShot=%d BulletClass=%s PoolSubsystem=%p"),
		UseOneShotBullet ? 1 : 0, *BulletClassName, PoolSubsystem), bDebugOnScreen, DebugDuration);
	if (!BulletToFire) return;
	
	ABullet* SpawnedBullet = nullptr;

	// Acquire from pool via cached subsystem (fast path)
	if (PoolSubsystem)
	{
		AActor* PooledActor = PoolSubsystem->AcquireActor(BulletToFire);
		SpawnedBullet = Cast<ABullet>(PooledActor);
		Debug::Log(FString::Printf(TEXT("[Combat] Acquire returned %p (cast to ABullet %p)"), PooledActor, SpawnedBullet), bDebugOnScreen, DebugDuration);
	}

	// Fallback to spawning if pooling is unavailable or failed
	if (!SpawnedBullet)
	{
		Debug::Log(TEXT("[Combat] Pool acquire failed; spawning new bullet"), bDebugOnScreen, DebugDuration);
		SpawnedBullet = GetWorld()->SpawnActor<ABullet>(
			BulletToFire,
			FirePoint.GetLocation(),
			ShootRotation,
			SpawnParams
			);
		if (!SpawnedBullet)
		{
			Debug::Log(TEXT("[Combat] SpawnActor failed for bullet"), bDebugOnScreen, DebugDuration);
			return;
		}
	}

	// Per-shot placement and ownership
	SpawnedBullet->SetActorLocationAndRotation(FirePoint.GetLocation(), ShootRotation);
	SpawnedBullet->SetOwner(GetOwner());
	SpawnedBullet->SetInstigator(SpawnParams.Instigator);

	float FireDamage = FallbackFireDamage;
	if (AttributeComponent) FireDamage = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Weapon_Damage);
	
	SpawnedBullet->Init(CombatEventsSubsystem, FireDamage, BulletSpeed);
	const FString SpawnedName = GetNameSafe(SpawnedBullet);
	Debug::Log(FString::Printf(TEXT("[Combat] Bullet initialized: %s (%p) Damage=%.2f Speed=%.2f"),
		*SpawnedName, SpawnedBullet, FireDamage, BulletSpeed), bDebugOnScreen, DebugDuration);
}


//Setters
void UCombatComponent::SetBullet(TSubclassOf<ABullet> Bullet)
{
	StandardBullet = Bullet;
	// Register pool for new bullet type to avoid runtime cost
	if (PoolSubsystem && StandardBullet)
	{
		PoolSubsystem->RegisterPool(StandardBullet, BulletPoolInitialSize);
	}
}

void UCombatComponent::SetOneShotBullet(TSubclassOf<ABullet> Bullet)
{
	OneShotBullet = Bullet;
	UseOneShotBullet = true;
	if (PoolSubsystem && OneShotBullet)
	{
		PoolSubsystem->RegisterPool(OneShotBullet, FMath::Max(1, BulletPoolInitialSize / 4));
	}
}

void UCombatComponent::ResetOneShotBullet()
{
	OneShotBullet = nullptr;
	UseOneShotBullet = false;
}


//Getters
USkeletalMeshComponent* UCombatComponent::GetGunMeshComp()
{
	return GunMeshComp;
}

FName UCombatComponent::GetFirePointSocketName()
{
	return GunMeshFirePointSocket;
}

FTransform UCombatComponent::GetFirePoint()
{
	return GunMeshComp->GetSocketTransform(GunMeshFirePointSocket);
}


//Context --> Input
void UCombatComponent::SetFireContext(bool WantsToFire)
{
	InputContext.bWantsToFire = WantsToFire;
}

void UCombatComponent::SetMeleeContext(bool WantsToMelee)
{
	InputContext.bWantsToMelee = WantsToMelee;
}

void UCombatComponent::SetAbilityContext(bool WantsToAbility)
{
	InputContext.bWantsToAbility = WantsToAbility;
}
