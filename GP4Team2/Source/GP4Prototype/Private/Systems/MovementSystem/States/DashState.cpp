#include "Systems/MovementSystem/States/DashState.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "Components/CapsuleComponent.h"
#include "Core/Data/Enums/GameDamageType.h"
#include "Core/Data/Interfaces/Damageable.h"
#include "Core/Subsystems/LookTraceSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Systems/MovementSystem/CustomCharacterMovementComponent.h"
#include "Systems/MovementSystem/MovementFiniteStateMachine.h"
#include "Systems/MovementSystem/States/RegularVaultState.h"
#include "Systems/MovementSystem/States/SlideVaultState.h"
#include "Systems/MovementSystem/States/WalkState.h"

class ULookTraceSubsystem;

void UDashState::Init(float InputFallbackDashSpeed, float InputDashDuration, float InitAccelerationDuringDash, float InputDashFallSpeedInAir,
					  float InputDashGravityScale, bool InCanDashInAir, float InitVaultTraceLength, float InitVaultTraceRadius,
					  ECollisionChannel InitVaultCollisionChannel, float InitVaultHeightRequirement, FName InitRegularVaultableTagName,
					  FName InitSlideVaultableTagName, int InMaxDashHitCapacity, float InDashHitDamage,
					  float InFallbackForwardHitKnockbackStrengthForSurvivingEnemies, float InUpwardHitKnockbackStrengthForSurvivingEnemies,
					  float InFallbackForwardHitKnockbackStrengthForDyingEnemies)
{
	// uhhh
	MovementFSM = Cast<UMovementFiniteStateMachine>(FSM);
	if (!MovementFSM) return;

	CustomCharMoveComp = MovementFSM->GetOwnerCustomCharacterMovementComponent();

	OwnerActor = MovementFSM->GetOwnerActor();
	
	if (OwnerActor) CharMoveComp = OwnerActor->GetComponentByClass<UCharacterMovementComponent>();
	if (OwnerActor) OwnerCapsuleComponent = OwnerActor->GetComponentByClass<UCapsuleComponent>();
	

	// set LookTraceSubsystem
	OwnerPawn = MovementFSM->GetOwnerPawn();
	if (!OwnerPawn) return;
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	
	LookTraceSubsystem = LocalPlayer->GetSubsystem<ULookTraceSubsystem>();


	// attribute component and stats
	if (OwnerActor) AttributeComponent = OwnerActor->GetComponentByClass<UAttributeComponent>();
	
	FallBackDashSpeed = InputFallbackDashSpeed;
	DashDuration = InputDashDuration;
	AccelerationDuringDash = InitAccelerationDuringDash;
	DashFallSpeedInAir = InputDashFallSpeedInAir;
	DashGravityScale = InputDashGravityScale;
	bCanDashInAir = InCanDashInAir;
	
	VaultTraceLength = InitVaultTraceLength;
	VaultTraceRadius = InitVaultTraceRadius;
	VaultCollisionChannel = InitVaultCollisionChannel;
	VaultHeightRequirement = InitVaultHeightRequirement;
	RegularVaultableTagName = InitRegularVaultableTagName;
	SlideVaultableTagName = InitSlideVaultableTagName;
	
	MaxDashHitCapacity = InMaxDashHitCapacity;
	FallbackDashHitDamage = InDashHitDamage;
	FallbackForwardHitKnockbackStrengthForSurvivingEnemies = InFallbackForwardHitKnockbackStrengthForSurvivingEnemies;
	UpwardHitKnockbackStrengthForSurvivingEnemies = InUpwardHitKnockbackStrengthForSurvivingEnemies;
	FallbackForwardHitKnockbackStrengthForDyingEnemies = InFallbackForwardHitKnockbackStrengthForDyingEnemies;
}

bool UDashState::Enter(FMovementContext Context) //something else needs to be passed in here to check cooldown
{	
	if (!Context.bCanDash) return false;
	if (!CharMoveComp) return false;
	if (!CustomCharMoveComp) return false;
	if (!bCanDashInAir && CharMoveComp->IsFalling()) return false;
		
	InitialAcceleration = CharMoveComp->MaxAcceleration;
	CharMoveComp->MaxAcceleration = AccelerationDuringDash;
	
	InitialDashFallSpeedInAir = CharMoveComp->BrakingDecelerationFalling;
	CharMoveComp->BrakingDecelerationFalling = DashFallSpeedInAir;

	InitialDashGravityScale = CharMoveComp->GravityScale;
	CharMoveComp->GravityScale = DashGravityScale;

	bool DidDash = StartDash(Context.MoveInput);
	if (!DidDash) return false;

	CustomCharMoveComp->OnDashStart.Broadcast();
	CustomCharMoveComp->OnDashExecuted();
	
	return true;
}

void UDashState::Tick(float DeltaTime, FMovementContext Context)
{
	Super::Tick(DeltaTime, Context);

	Dash();

	if (Context.bMeleeDashUnlocked) TryDashHit();

	if (CurrentDashTime < DashDuration) CurrentDashTime += DeltaTime;
	else FSM->SetState(UWalkState::StaticClass(), Context);

	if (TryVaultTransition(Context)) return; //maybe every 0.05 sec instead?
}

bool UDashState::Exit()
{
	StopDash();

	CharMoveComp->MaxAcceleration = InitialAcceleration;
	CharMoveComp->BrakingDecelerationFalling = InitialDashFallSpeedInAir;
	CharMoveComp->GravityScale = InitialDashGravityScale;

	CharMoveComp->StopMovementImmediately();

	CustomCharMoveComp->OnDashFinish.Broadcast();
		
	return Super::Exit();
}

bool UDashState::StartDash(FVector2f MoveInput)
{
	if (!OwnerActor) return false;
	
	FVector ForwardBack = OwnerActor->GetActorForwardVector() * MoveInput.Y;
	FVector LeftRight = OwnerActor->GetActorRightVector() * MoveInput.X;
	ResultDashDir = (ForwardBack+LeftRight).GetSafeNormal();

	if (ResultDashDir.Size() <= 0) return false;

	if (!CharMoveComp) return false;

	InitialSpeed = CharMoveComp->GetMaxSpeed();

	float MoveSpeed = FallBackDashSpeed;
	if (AttributeComponent) MoveSpeed = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Dash_Strength);

	CharMoveComp->MaxWalkSpeed = MoveSpeed;

	CurrentDashTime = 0;
	DamagedActors.Empty();
	
	return true;
}

void UDashState::Dash()
{
	OwnerPawn->AddMovementInput(ResultDashDir);
}

bool UDashState::StopDash()
{
	CharMoveComp->MaxWalkSpeed = InitialSpeed;
	
	return true;
}

bool UDashState::TryVaultTransition(FMovementContext Context)
{
	FHitResult Hit = LookTraceSubsystem->GetHitResultFromPawnForwardSphereTrace(
		OwnerPawn, VaultTraceLength, VaultTraceRadius, VaultCollisionChannel);

	AActor* VaultableObject = Hit.GetActor();
	
	if (!VaultableObject) return false;
	if (!VaultableObject->ActorHasTag(RegularVaultableTagName) && !VaultableObject->ActorHasTag(SlideVaultableTagName)) return false;

	
	// calcualte VaultableObjectHeight + SetSlideStartLoc
	FVector TargetOrigin;
	FVector TargetExtents;
	VaultableObject->GetActorBounds(true, TargetOrigin, TargetExtents);

	FVector TargetDir = OwnerPawn->GetActorLocation() - TargetOrigin;
	TargetDir.Z = 0.0f;
	
	FVector SlideStartLocation = TargetOrigin;
	SlideStartLocation.X += FMath::Clamp(TargetDir.X, -TargetExtents.X, TargetExtents.X);
	SlideStartLocation.Y += FMath::Clamp(TargetDir.Y, -TargetExtents.Y, TargetExtents.Y);

	float ObjectTopZ = TargetOrigin.Z + TargetExtents.Z;
	SlideStartLocation.Z = ObjectTopZ + 100;

	FVector PlayerFeetLocation = OwnerCapsuleComponent->GetComponentLocation() - FVector(0, 0, OwnerCapsuleComponent->GetScaledCapsuleHalfHeight());
	float ObjectHeight = ObjectTopZ - PlayerFeetLocation.Z;
	
	if (VaultHeightRequirement < ObjectHeight) return false;
	
	Context.VaultStartLocation = SlideStartLocation;

	if (VaultableObject->ActorHasTag(RegularVaultableTagName)) FSM->SetState(URegularVaultState::StaticClass(), Context);
	else                                                       FSM->SetState(USlideVaultState::StaticClass(), Context);
	
	return true;
}



void UDashState::TryDashHit()
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

	float DashHitDamage = FallbackDashHitDamage;
	if (AttributeComponent) DashHitDamage = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Dash_CollisionDamage);
	
	IDamageable::Execute_TakeDamage(HitActor, EGameDamageType::DashHit, DashHitDamage, 1.0f, "");
	if (!HitActor) return;
	
	DamagedActors.Add(HitActor);
	
	//move these to enemy itself
	KnockbackLiving(HitActor);
	KnockbackDead(HitActor);
}

void UDashState::KnockbackLiving(AActor* HitActor)
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

void UDashState::KnockbackDead(AActor* HitActor)
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
