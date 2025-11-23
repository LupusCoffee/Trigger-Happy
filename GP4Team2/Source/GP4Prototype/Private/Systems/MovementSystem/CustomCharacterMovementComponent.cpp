#include "GP4Prototype/Public/Systems/MovementSystem/CustomCharacterMovementComponent.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GP4Prototype/Public/Core/ReusableSystems/FiniteStateMachine/BaseMovementFiniteStateMachine.h"
#include "GP4Prototype/Public/Systems/MovementSystem/MovementFiniteStateMachine.h"
#include "GP4Prototype/Public/Systems/MovementSystem/States/WalkState.h"
#include "Systems/MovementSystem/States/CrouchState.h"
#include "Systems/MovementSystem/States/DashState.h"
#include "Systems/MovementSystem/States/RegularVaultState.h"
#include "Systems/MovementSystem/States/SlideState.h"
#include "Systems/MovementSystem/States/SprintState.h"
#include "Systems/MovementSystem/States/SlideVaultState.h"

UCustomCharacterMovementComponent::UCustomCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UCustomCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();


	//Initial Setting
	Owner = GetOwner();
	if (!Owner) return;

	Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	if (Owner) AttributeComponent = Owner->GetComponentByClass<UAttributeComponent>();

	
	//Initial Value
	float DashCharges = FallbackMaxDashCharges;
	if (AttributeComponent) DashCharges = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Dash_MaxCharges);
	
	CurrentDashCharges = DashCharges;
	CurrentDashCooldownTime = 0;
	CurrentSlideCooldownTime = 0;
	CurrentVaultCooldownTime = 0;

	InputContext.bMeleeDashUnlocked = false;
	InputContext.bMeleeSlideUnlocked = false;


	//Movement Finite State Machine
	// fsm setup
	MovementFSM = NewObject<UMovementFiniteStateMachine>();
	MovementFSM->Init(Owner, Pawn, this);
	
	UWalkState* WalkState = NewObject<UWalkState>();
	USprintState* SprintState = NewObject<USprintState>();
	UCrouchState* CrouchState = NewObject<UCrouchState>();
	UDashState* DashState = NewObject<UDashState>();
	USlideState* SlideState = NewObject<USlideState>();
	URegularVaultState* RegularVaultState = NewObject<URegularVaultState>();
	USlideVaultState* SlideVaultState = NewObject<USlideVaultState>();
	
	MovementFSM->SetupStates({
		WalkState,
		SprintState,
		CrouchState,
		DashState,
		SlideState,
		RegularVaultState,
		SlideVaultState
	});

	WalkState->Init(FallbackWalkSpeed);
	SprintState->Init(FallbackSprintSpeed, VaultTraceLength, VaultTraceRadius, MovementTraceChannel,
					  SprintToVaultHeightRequirement, RegularVaultableTagName, SlideVaultableTagName);
	CrouchState->Init(CrouchDownSpeed, UncrouchSpeed, FallbackCrouchSpeed, CrouchHeightMultiplier);
	DashState->Init(FallbackDashSpeed, DashDuration, AccelerationDuringDash, DashFallSpeedInAir, DashGravityScale,
					bCanDashInAir, VaultTraceLength, VaultTraceRadius, MovementTraceChannel,
					DashToVaultHeightRequirement, RegularVaultableTagName, SlideVaultableTagName,
					MaxDashHitCapacity, FallbackDashHitDamage, FallbackForwardHitKnockbackStrengthForSurvivingEnemies,
					UpwardHitKnockbackStrengthForSurvivingEnemies, FallbackForwardHitKnockbackStrengthForDyingEnemies);
	SlideState->Init(FallbackSlideStrength, SlideFrictionLevel, FallbackSlideDuration, SlideHeightMultiplier,
					MaxSlideHitCapacity, FallbackSlideHitDamage, FallbackForwardSlideHitKnockbackStrengthForSurvivingEnemies,
				    UpwardSlideHitKnockbackStrengthForSurvivingEnemies, FallbackForwardSlideHitKnockbackStrengthForDyingEnemies,
				    SlideHitTraceLength, SlideHitTraceRadius);
	RegularVaultState->Init();
	SlideVaultState->Init(SlideVaultStrength, SlideVaultFrictionLevel, SlideVaultHeightMultiplier, SlideVaultableTagName, MovementTraceChannel);
	
	// inital state set
	MovementFSM->SetState(UWalkState::StaticClass(), InputContext);


	//Components
	if (AActor* OwnerActor = MovementFSM->GetOwnerActor())
	{
		UCharacterMovementComponent* CharMoveComp = OwnerActor->GetComponentByClass<UCharacterMovementComponent>();
		CharMoveComp->BrakingDecelerationFalling = FallSpeedInAir;
		CharMoveComp->GravityScale = GravityScale;
	}

	// Subscribe to AttributeValue Changed for max charges to increase current charges by the positive delta of the change
	//Subscribe to Events
	AttributeComponent->OnAttributeValueChanged.AddDynamic(this, &UCustomCharacterMovementComponent::OnAttributeValueChanged);
}

void UCustomCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickDashCooldown(DeltaTime);
	TickSlideCooldown(DeltaTime);
	TickVaultCooldown(DeltaTime);
	MovementFSM->Tick(DeltaTime, InputContext);
}

void UCustomCharacterMovementComponent::TickDashCooldown(float DeltaTime)
{
	float CooldownTime = FallbackDashCooldownPerChargeTime;
	if (AttributeComponent) CooldownTime = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Dash_CooldownPerCharge);

	float DashCharges = FallbackMaxDashCharges;
	if (AttributeComponent) DashCharges = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Dash_MaxCharges);

	if (CurrentDashCharges < DashCharges)
	{
		if (CurrentDashCooldownTime < CooldownTime) CurrentDashCooldownTime += DeltaTime;
		else
		{
			CurrentDashCharges++;
			CurrentDashCooldownTime = 0;

			DashChargeReplenish.Broadcast();
		}
	}

	if (CurrentDashCharges > 0) InputContext.bCanDash = true;
	else InputContext.bCanDash = false;

	DashContextUpdate.Broadcast(CurrentDashCooldownTime, CooldownTime, CurrentDashCharges, DashCharges);
}

void UCustomCharacterMovementComponent::TickSlideCooldown(float DeltaTime)
{
	float SlideCooldownTime = FallbackSlideCooldownTime;
	if (AttributeComponent) SlideCooldownTime = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Slide_Cooldown);
	
	if (CurrentSlideCooldownTime < SlideCooldownTime)
	{
		InputContext.bCanSlide = false;
		CurrentSlideCooldownTime += DeltaTime;
	}
	else InputContext.bCanSlide = true;
}

void UCustomCharacterMovementComponent::TickVaultCooldown(float DeltaTime)
{
	if (CurrentVaultCooldownTime < SlideVaultCooldownTime)
	{
		InputContext.bCanVault = false;
		CurrentVaultCooldownTime += DeltaTime;
	}
	else InputContext.bCanVault = true;
}


// unlock abilities
void UCustomCharacterMovementComponent::UnlockMeleeDash()
{
	InputContext.bMeleeDashUnlocked = true;
}

void UCustomCharacterMovementComponent::UnlockMeleeSlide()
{
	InputContext.bMeleeSlideUnlocked = true;
}

// input context functions
void UCustomCharacterMovementComponent::SetMoveContext(FVector2f MoveInput)
{
	InputContext.MoveInput = MoveInput;
}

void UCustomCharacterMovementComponent::SetSprintContext(bool WantsToSprint)
{
	InputContext.bWantsToSprint = WantsToSprint;
}

void UCustomCharacterMovementComponent::SetCrouchContext(bool WantsToCrouch)
{
	InputContext.bWantsToCrouch = WantsToCrouch;
}

void UCustomCharacterMovementComponent::SetDashContext(bool WantsToDash)
{
	InputContext.bWantsToDash = WantsToDash;
	if (CurrentDashCharges <= 0) InputContext.bWantsToDash = false;
}


// value context functions
void UCustomCharacterMovementComponent::OnDashExecuted()
{
	CurrentDashCharges--;
	InputContext.bWantsToDash = false;	//really dont wanna set this here hmmm
}

void UCustomCharacterMovementComponent::OnSlideExecuted()
{
	CurrentSlideCooldownTime = 0;
}

void UCustomCharacterMovementComponent::OnSlideVaultExecuted()
{
	CurrentVaultCooldownTime = 0;
}


// event subscriptions
void UCustomCharacterMovementComponent::OnAttributeValueChanged(FGameplayTag AttributeTag, float OldValue, float NewValue)
{
	if (AttributeTag == AttributeTags::Attribute_Dash_MaxCharges)
	{
		CurrentDashCharges = NewValue;
	}
}