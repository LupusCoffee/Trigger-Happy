#include "Systems/MovementSystem/States/SlideState.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "Components/CapsuleComponent.h"
#include "Core/Data/Enums/GameDamageType.h"
#include "Core/Data/Interfaces/Damageable.h"
#include "Core/Subsystems/LookTraceSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Systems/MovementSystem/CustomCharacterMovementComponent.h"
#include "Systems/MovementSystem/MovementFiniteStateMachine.h"
#include "Systems/MovementSystem/States/CrouchState.h"
#include "Systems/MovementSystem/States/DashState.h"
#include "Systems/MovementSystem/States/WalkState.h"

void USlideState::Init(float InitFallbackSlideStrength, float InitSlideFrictionLevel, float InitFallbackSlideDuration, float InitSlideHeightMultiplier,
						int InMaxDashHitCapacity, float InDashHitDamage, float InFallbackForwardHitKnockbackStrengthForSurvivingEnemies,
				       float InUpwardHitKnockbackStrengthForSurvivingEnemies, float InFallbackForwardHitKnockbackStrengthForDyingEnemies,
				        float InitVaultTraceLength, float InitVaultTraceRadius)
{
	//this whole function is quite messy --> FIIIIIIIIIIIIX WOOOOOOOOOOOOOOOOOOOOOOOOOOOO
	UMovementFiniteStateMachine* MovementFSM = Cast<UMovementFiniteStateMachine>(FSM);
	if (!MovementFSM) return;

	
	FallbackSlideStrength = InitFallbackSlideStrength;
	SlideFrictionLevel = InitSlideFrictionLevel;
	FallbackSlideDuration = InitFallbackSlideDuration;
	
	VaultTraceLength = InitVaultTraceLength;
	VaultTraceRadius = InitVaultTraceRadius;

	MaxDashHitCapacity = InMaxDashHitCapacity;
	FallbackSlideHitDamage = InDashHitDamage;
	FallbackForwardHitKnockbackStrengthForSurvivingEnemies = InFallbackForwardHitKnockbackStrengthForSurvivingEnemies;
	UpwardHitKnockbackStrengthForSurvivingEnemies = InUpwardHitKnockbackStrengthForSurvivingEnemies;
	FallbackForwardHitKnockbackStrengthForDyingEnemies = InFallbackForwardHitKnockbackStrengthForDyingEnemies;


	CustomCharMoveComp = MovementFSM->GetOwnerCustomCharacterMovementComponent();
	
	if (AActor* OwnerActor = MovementFSM->GetOwnerActor())
	{
		OwnerCharacterMovementComponent = OwnerActor->GetComponentByClass<UCharacterMovementComponent>();
		
		OwnerCapsuleComponent = OwnerActor->GetComponentByClass<UCapsuleComponent>();
		if (OwnerCapsuleComponent)
		{
			InitialCapsuleHalfHeight = OwnerCapsuleComponent->GetUnscaledCapsuleHalfHeight();
			CurrentCapsuleHalfHeight = InitialCapsuleHalfHeight * InitSlideHeightMultiplier;
		}
	}

	
	OwnerPawn = MovementFSM->GetOwnerPawn();

	if (OwnerPawn) AttributeComponent = OwnerPawn->GetComponentByClass<UAttributeComponent>();
	
	
	// set LookTraceSubsystem
	OwnerPawn = MovementFSM->GetOwnerPawn();
	if (!OwnerPawn) return;
    	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;
    
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
    	
	LookTraceSubsystem = LocalPlayer->GetSubsystem<ULookTraceSubsystem>();
}

bool USlideState::Enter(FMovementContext Context)
{
	if (!Context.bCanSlide) return false;
	
	// Reset transition flag on enter
	bPreserveLoweredHeightOnExit = false;

	StartSlide();
	CustomCharMoveComp->OnSlideStart.Broadcast();
	
	//set height
	if (OwnerCapsuleComponent)
	{
		OwnerCapsuleComponent->SetCapsuleHalfHeight(CurrentCapsuleHalfHeight);

		GroundOffset = (InitialCapsuleHalfHeight - CurrentCapsuleHalfHeight)/2;

		FVector NewLocation = FVector(
			OwnerPawn->GetActorLocation().X,
			OwnerPawn->GetActorLocation().Y,
			OwnerPawn->GetActorLocation().Z - GroundOffset);
		
		OwnerPawn->SetActorLocation(NewLocation);
	}
	
	return true;
}

bool USlideState::Exit()
{
	StopSlide();
	CustomCharMoveComp->OnSlideFinish.Broadcast();
	
	// Only restore standing height if we're NOT going to Crouch
	if (!bPreserveLoweredHeightOnExit)
	{
		//set crouch height back to initial
		if (OwnerCapsuleComponent) OwnerCapsuleComponent->SetCapsuleHalfHeight(InitialCapsuleHalfHeight);

		FVector NewLocation = FVector(
				OwnerPawn->GetActorLocation().X,
				OwnerPawn->GetActorLocation().Y,
				OwnerPawn->GetActorLocation().Z + GroundOffset);

		OwnerPawn->SetActorLocation(NewLocation);
	}

	CustomCharMoveComp->OnSlideExecuted();

	// Clear for safety
	bPreserveLoweredHeightOnExit = false;
	
	return true;
}

void USlideState::Tick(float DeltaTime, FMovementContext Context)
{
	Super::Tick(DeltaTime, Context);

	if (Context.bMeleeSlideUnlocked) TryDashHit();

	float SlideDuration = FallbackSlideDuration;
	if (AttributeComponent) SlideDuration = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Slide_Duration);
	
	if (CurrentSlideDuration < SlideDuration) CurrentSlideDuration += DeltaTime;
	else
	{
		// Going to Crouch: keep the lowered height
		bPreserveLoweredHeightOnExit = true;
		FSM->SetState(UCrouchState::StaticClass(), Context);
	}
	
	if (!Context.bWantsToCrouch)
	{
		// Going to Crouch: keep the lowered height
		bPreserveLoweredHeightOnExit = true;
		FSM->SetState(UCrouchState::StaticClass(), Context);
	}
	if (Context.bWantsToDash)
	{
		// Not going to crouch; restore standing height on exit
		bPreserveLoweredHeightOnExit = false;
		FSM->SetState(UDashState::StaticClass(), Context);
	}
}

bool USlideState::StartSlide()
{
	if (!OwnerPawn) return false;
	if (!OwnerCharacterMovementComponent) return false;

	InitialFrictionLevel = OwnerCharacterMovementComponent->GroundFriction;
	OwnerCharacterMovementComponent->GroundFriction = SlideFrictionLevel;
	
	FVector ForwardBack = OwnerPawn->GetActorForwardVector();
	FVector ResultMoveDir = (ForwardBack).GetSafeNormal();

	float SlideStrength = FallbackSlideStrength;
	if (AttributeComponent) SlideStrength = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Slide_Strength);
	
	OwnerCharacterMovementComponent->Launch(ResultMoveDir * SlideStrength);

	CurrentSlideDuration = 0;
	DamagedActors.Empty();

	return true;
}

bool USlideState::StopSlide()
{
	OwnerCharacterMovementComponent->GroundFriction = InitialFrictionLevel;
	
	return true;
}


void USlideState::TryDashHit()
{
	//check if unlocked from movement comp?
	
	if (!LookTraceSubsystem) return;
	
	FHitResult Hit = LookTraceSubsystem->GetHitResultFromPawnForwardSphereTraceWithOffset(
		100, OwnerPawn, VaultTraceLength, VaultTraceRadius, ECollisionChannel::ECC_Camera);
	
	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return;
	if (HitActor == CustomCharMoveComp->GetOwner()) return;
	if (!HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass())) return;

	if (DamagedActors.Contains(HitActor)) return;

	if (DamagedActors.Num() >= MaxDashHitCapacity) return;


	float SladeHitDamage = FallbackSlideHitDamage;
	if (AttributeComponent) SladeHitDamage = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Slide_CollisionDamage);
	
	IDamageable::Execute_TakeDamage(HitActor, EGameDamageType::SlideHit, SladeHitDamage, 1.0f, "");
	if (!HitActor) return;
	
	DamagedActors.Add(HitActor);
	
	//move these to enemy itself
	KnockbackLiving(HitActor);
	KnockbackDead(HitActor);
}

void USlideState::KnockbackLiving(AActor* HitActor)
{
	// sigh --> todo: need a way to get the character easily --> do it in the enemies instead

	if (!HitActor) return;
	ACharacter* Char = Cast<ACharacter>(HitActor);
	if (Char)
	{
		// Direction from attacker -> victim (mostly horizontal)
		FVector Dir = (Char->GetActorLocation() - CustomCharMoveComp->GetOwner()->GetActorLocation());
		Dir.Z = 0.f;
		Dir = Dir.GetSafeNormal();
		
		
		float ForwardMeleeKnockbackStrengthForSurvivingEnemies = FallbackForwardHitKnockbackStrengthForSurvivingEnemies;
		if (AttributeComponent) ForwardMeleeKnockbackStrengthForSurvivingEnemies = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Melee_KnockbackForce);
		
		const float HorizontalStrength = ForwardMeleeKnockbackStrengthForSurvivingEnemies;
		const float VerticalBoost      = UpwardHitKnockbackStrengthForSurvivingEnemies;
		const FVector LaunchVel = (Dir * HorizontalStrength) + FVector(0,0,VerticalBoost);
		
		
		// Add to current velocity (don’t overwrite):
		const bool bXYOverride = false;
		const bool bZOverride  = false;
		Char->LaunchCharacter(LaunchVel, bXYOverride, bZOverride);
	}
}

void USlideState::KnockbackDead(AActor* HitActor)
{
	if (!HitActor) return;
	if (!CustomCharMoveComp) return;

	ACharacter* Char = Cast<ACharacter>(HitActor);
	if (!Char) return;
	USkeletalMeshComponent* SkeletalMeshComponent = Char->GetMesh();
	if (!SkeletalMeshComponent) return;

	
	float ForwardMeleeKnockbackStrengthForDyingEnemies = FallbackForwardHitKnockbackStrengthForDyingEnemies;
	if (AttributeComponent) ForwardMeleeKnockbackStrengthForDyingEnemies = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Melee_KnockbackForceDead);

	FVector Direction = (HitActor->GetActorLocation() - CustomCharMoveComp->GetOwner()->GetActorLocation()).GetSafeNormal();
	SkeletalMeshComponent->AddImpulseToAllBodiesBelow(Direction * ForwardMeleeKnockbackStrengthForDyingEnemies, NAME_None, true);
}