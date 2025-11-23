#include "Systems/CombatSystem/States/MeleeState.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "Core/Data/Enums/GameDamageType.h"
#include "Core/Data/Interfaces/Damageable.h"
#include "GameFramework/Character.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "Systems/CombatSystem/CombatComponent.h"
#include "Systems/CombatSystem/CombatFiniteStateMachine.h"
#include "Systems/CombatSystem/Components/HealthComponent.h"
#include "Systems/CombatSystem/Misc/MeleeEndAnimNotify.h"
#include "Systems/CombatSystem/Misc/MeleeStartAnimNotify.h"
#include "Systems/CombatSystem/States/IdleState.h"


//Init and Setup
void UMeleeState::Init( float InitFallbackMeleeDamage,
						float InitFallbackForwardMeleeKnockbackStrengthForSurvivingEnemies,
						float InitUpwardMeleeKnockbackStrengthForSurvivingEnemies,
						float InitFallbackForwardMeleeKnockbackStrengthForDyingEnemies, float InitMeleeForwardStartOffset, float InitMeleeRadius,
						float InitMeleeReach, float InitMeleeWidth,
						ECollisionChannel InitMeleeTraceChannel, float InitFallbackMaxMeleeHitCapacity,
						float InitMeleeDuration, UAnimMontage* InitBatMeleeMontage, UStaticMeshComponent* InitBatMeshComp,
						USkeletalMeshComponent* InitGunMeshComp, FName InBatBottomSocket, FName InBatTipSocket,
						FName InMeleeStartNotify,
						FName InMeleeEndNotify)
{
	// set combat fsm
	CombatFSM = Cast<UCombatFiniteStateMachine>(FSM);
	if (!CombatFSM) return;

	
	// set combat comp
	OwnerActor = CombatFSM->GetOwnerActor();
	if (!OwnerActor) return;
	
	CombatComponent = OwnerActor->GetComponentByClass<UCombatComponent>();


	// set look trace subsystem
	APawn* OwnerPawn = CombatFSM->GetOwnerPawn();
	if (!OwnerPawn) return;
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	
	LookTraceSubsystem = LocalPlayer->GetSubsystem<ULookTraceSubsystem>();


	// set controller
	Controller = OwnerPawn->GetController();


	// set anim instance
	USkeletalMeshComponent* SkelMesh = CombatComponent->GetSkelMeshByTag("CharArms");
	if (!SkelMesh) return;
	AnimInstance = SkelMesh->GetAnimInstance();


	// initial setting of variables + attribute comp
	if (OwnerPawn) AttributeComponent = OwnerPawn->GetComponentByClass<UAttributeComponent>();
	
	FallbackMeleeDamage = InitFallbackMeleeDamage;
	FallbackForwardMeleeKnockbackStrengthForSurvivingEnemies = InitFallbackForwardMeleeKnockbackStrengthForSurvivingEnemies;
	UpwardMeleeKnockbackStrengthForSurvivingEnemies = InitUpwardMeleeKnockbackStrengthForSurvivingEnemies;
	FallbackForwardMeleeKnockbackStrengthForDyingEnemies = InitFallbackForwardMeleeKnockbackStrengthForDyingEnemies;
	MeleeForwardStartOffset = InitMeleeForwardStartOffset;
	MeleeRadius = InitMeleeRadius;
	MeleeReach = InitMeleeReach;
	MeleeWidth = InitMeleeWidth;
	MeleeTraceChannel = InitMeleeTraceChannel;
	FallbackMaxMeleeHitCapacity = InitFallbackMaxMeleeHitCapacity;
	MeleeDuration = InitMeleeDuration;
	
	BatMeleeMontage = InitBatMeleeMontage;
	BatMeshComp = InitBatMeshComp;
	GunMeshComp = InitGunMeshComp;
	BatBottomSocket = InBatBottomSocket;
	BatTipSocket = InBatTipSocket;
	MeleeStartNotify = InMeleeStartNotify;
	MeleeEndNotify = InMeleeEndNotify;


	// figure out melee active duration
	if (!BatMeleeMontage) return;
	
	MeleeActiveStartTime = 0;
	MeleeActiveEndTime = 0;

	float PlayRate = BatMeleeMontage->GetPlayLength() / MeleeDuration;

	for (auto AnimNotifyEvent : BatMeleeMontage->Notifies)
	{
		if (AnimNotifyEvent.Notify.GetClass() == UMeleeStartAnimNotify::StaticClass()) MeleeActiveStartTime = AnimNotifyEvent.GetTriggerTime() / PlayRate;
		if (AnimNotifyEvent.Notify.GetClass() == UMeleeEndAnimNotify::StaticClass()) MeleeActiveEndTime = AnimNotifyEvent.GetTriggerTime() / PlayRate;
	}
	
	MeleeActiveDuration = MeleeActiveEndTime - MeleeActiveStartTime;
}


//Enter, Tick, Exit
bool UMeleeState::Enter(FCombatContext Context)
{
	if (!Context.bCanMelee) return false;
	if (!CombatComponent) return false;
	if (!AnimInstance) return false;
	if (!BatMeleeMontage) return false;
	
	CombatComponent->OnMeleeStarted();
	CombatComponent->OnMeleeStart.Broadcast();
	MeleeCurrentTime = 0;
	MeleeActiveCurrentTime = 0;
	DamagedActors.Empty();
	bMeleeActive = false;

	//melee start
	BatMeshComp->SetVisibility(true);
	GunMeshComp->SetVisibility(false);
	
	float PlayRate = BatMeleeMontage->GetPlayLength() / MeleeDuration;
    AnimInstance->Montage_Play(BatMeleeMontage, PlayRate);
	
	return true;
}

void UMeleeState::Tick(float DeltaTime, FCombatContext Context)
{
	Super::Tick(DeltaTime, Context);
	
	if (MeleeCurrentTime >= MeleeActiveStartTime && MeleeCurrentTime <= MeleeActiveEndTime)
	{
		//hit logic
		FVector CurrentBase = BatMeshComp->GetSocketLocation(BatTipSocket);

		FVector MidPoint = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * MeleeForwardStartOffset;
		
		FVector StartSwing = MidPoint - OwnerActor->GetActorRightVector()*MeleeWidth;
		FVector EndSwing   = MidPoint + OwnerActor->GetActorRightVector()*MeleeWidth;
		
		float Alpha = FMath::Clamp(MeleeActiveCurrentTime / MeleeActiveDuration, 0.07f, 1.0f);
		FVector CurrentLocation = FMath::Lerp(StartSwing, EndSwing, Alpha);

		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Location: %f"), Alpha));

		//shoot trace from CurrentBase, with player forward
		TArray<FHitResult> Hits = LookTraceSubsystem->GetHitResultFromLocationWithDirectionSphereTrace(OwnerActor->GetActorForwardVector(), CurrentLocation, MeleeReach, MeleeRadius, ECC_Camera);
		for (auto Hit : Hits) OnMeleeHit(Hit);

		MeleeActiveCurrentTime += DeltaTime;
	}

	MeleeCurrentTime += DeltaTime;
	if (MeleeCurrentTime >= MeleeDuration) FSM->SetState(UIdleState::StaticClass(), Context);
}

bool UMeleeState::Exit()
{
	if (BatMeshComp) BatMeshComp->SetVisibility(false);
	GunMeshComp->SetVisibility(true);
	
	if (CombatComponent)
	{
		CombatComponent->OnMeleeFinished();
		CombatComponent->OnMeleeFinish.Broadcast();
	}
	
	return true;
}

void UMeleeState::OnMeleeHit(FHitResult Hit)	//subscribe to OnHitComponent with this
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return;
	if (HitActor == OwnerActor) return;
	if (!HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass())) return;

	if (DamagedActors.Contains(HitActor)) return;

	
	int MaxMeleeHitCapacity = FallbackMaxMeleeHitCapacity;
	if (AttributeComponent) MaxMeleeHitCapacity = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Melee_HitDetection);

	if (DamagedActors.Num() >= MaxMeleeHitCapacity) return;

	
	float MeleeDamage = FallbackMeleeDamage;
	if (AttributeComponent) MeleeDamage = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Melee_Damage);
	
	IDamageable::Execute_TakeDamage(HitActor, EGameDamageType::Melee, MeleeDamage, 1.0f, "");
	if (!HitActor) return;
	
	DamagedActors.Add(HitActor);

	// get hit actor
	USkeletalMeshComponent* SkelMesh = nullptr;
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	HitActor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
	for (USkeletalMeshComponent* Skel : SkeletalMeshes)
	{
		if (!Skel || !Skel->ComponentHasTag("DamageableSkelMesh")) continue;
		SkelMesh = Skel;
	}
	CombatComponent->OnMeleeHit.Broadcast(SkelMesh);

	//move these to enemy
	KnockbackLiving(HitActor);
	KnockbackDead(HitActor);
}

void UMeleeState::KnockbackLiving(AActor* HitActor)
{
	// sigh --> todo: need a way to get the character easily --> do it in the enemies instead

	if (!HitActor) return;
	
	ACharacter* Char = Cast<ACharacter>(HitActor);
	if (Char)
	{
		// Direction from attacker -> victim (mostly horizontal)
		FVector Dir = (Char->GetActorLocation() - CombatComponent->GetOwner()->GetActorLocation());
		Dir.Z = 0.f;
		Dir = Dir.GetSafeNormal();

		
		float ForwardMeleeKnockbackStrengthForSurvivingEnemies = FallbackForwardMeleeKnockbackStrengthForSurvivingEnemies;
		if (AttributeComponent) ForwardMeleeKnockbackStrengthForSurvivingEnemies = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Melee_KnockbackForce);

		const float HorizontalStrength = ForwardMeleeKnockbackStrengthForSurvivingEnemies;
		const float VerticalBoost      = UpwardMeleeKnockbackStrengthForSurvivingEnemies;
		const FVector LaunchVel = (Dir * HorizontalStrength) + FVector(0,0,VerticalBoost);

		
		// Add to current velocity (don’t overwrite):
		const bool bXYOverride = false;
		const bool bZOverride  = false;
		Char->LaunchCharacter(LaunchVel, bXYOverride, bZOverride);
	}
}

void UMeleeState::KnockbackDead(AActor* HitActor)
{
	if (!HitActor) return;
	if (!CombatComponent) return;

	ACharacter* Char = Cast<ACharacter>(HitActor);
	if (!Char) return;
	USkeletalMeshComponent* SkeletalMeshComponent = Char->GetMesh();
	if (!SkeletalMeshComponent) return;

	
	float ForwardMeleeKnockbackStrengthForDyingEnemies = FallbackForwardMeleeKnockbackStrengthForDyingEnemies;
	if (AttributeComponent) ForwardMeleeKnockbackStrengthForDyingEnemies = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Melee_KnockbackForceDead);

	FVector Direction = (HitActor->GetActorLocation() - CombatComponent->GetOwner()->GetActorLocation()).GetSafeNormal();
	SkeletalMeshComponent->AddImpulseToAllBodiesBelow(Direction * ForwardMeleeKnockbackStrengthForDyingEnemies, NAME_None, true);
}
