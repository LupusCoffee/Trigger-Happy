#include "Systems/MovementSystem/States/SlideVaultState.h"
#include "Components/CapsuleComponent.h"
#include "Core/Subsystems/LookTraceSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Systems/MovementSystem/MovementFiniteStateMachine.h"
#include "Systems/MovementSystem/States/WalkState.h"

class ULookTraceSubsystem;

void USlideVaultState::Init(float InitSlideStrength, float InitSlideFrictionLevel, float InitSlideHeightMultiplier, FName InitVaultableTagName,
					   ECollisionChannel InitVaultTraceChannel)
{
	SlideStrength = InitSlideStrength;
	SlideFrictionLevel = InitSlideFrictionLevel;
	VaultableTagName = InitVaultableTagName;
	VaultTraceChannel = InitVaultTraceChannel;


	//CharacterMoveComp, etc.
	UMovementFiniteStateMachine* MovementFSM = Cast<UMovementFiniteStateMachine>(FSM);
	if (!MovementFSM) return;
	
	CustomCharMoveComp = MovementFSM->GetOwnerCustomCharacterMovementComponent();

	AActor* OwnerActor = MovementFSM->GetOwnerActor();
	if (!OwnerActor) return;

	OwnerCharacterMovementComponent = OwnerActor->GetComponentByClass<UCharacterMovementComponent>();

	
	//Owner Comp, initial slide scale, etc.
	OwnerCapsuleComponent = OwnerActor->GetComponentByClass<UCapsuleComponent>();
	if (OwnerCapsuleComponent)
	{
		InitialCapsuleHalfHeight = OwnerCapsuleComponent->GetUnscaledCapsuleHalfHeight();
		CurrentCapsuleHalfHeight = InitialCapsuleHalfHeight * InitSlideHeightMultiplier;
	}
	

	//Owner Pawn, etc.
	OwnerPawn = MovementFSM->GetOwnerPawn();


	// set LookTraceSubsystem
	if (!OwnerPawn) return;
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	
	LookTraceSubsystem = LocalPlayer->GetSubsystem<ULookTraceSubsystem>();
}

bool USlideVaultState::Enter(FMovementContext Context)
{
	if (!Context.bCanVault) return false;
	
	StartSlide(Context.VaultStartLocation);
	CustomCharMoveComp->OnSlideVaultStart.Broadcast();

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

void USlideVaultState::Tick(float DeltaTime, FMovementContext Context)
{
	Super::Tick(DeltaTime, Context);

	if (TryWalkTransition(Context)) return;
}

bool USlideVaultState::Exit()
{
	StopSlide();
	CustomCharMoveComp->OnSlideVaultFinish.Broadcast();

	//set crouch height back to initial
	if (OwnerCapsuleComponent) OwnerCapsuleComponent->SetCapsuleHalfHeight(InitialCapsuleHalfHeight);

	FVector NewLocation = FVector(
			OwnerPawn->GetActorLocation().X,
			OwnerPawn->GetActorLocation().Y,
			OwnerPawn->GetActorLocation().Z + GroundOffset);

	OwnerPawn->SetActorLocation(NewLocation);

	CustomCharMoveComp->OnSlideVaultExecuted();
	
	return true;
}

bool USlideVaultState::StartSlide(FVector SlideStartLocation)
{
	if (SlideStartLocation == FVector::ZeroVector) return false;
	if (!OwnerPawn) return false;
	if (!OwnerCharacterMovementComponent) return false;

	
	OwnerPawn->SetActorLocation(SlideStartLocation); //lerp to this position with an assignable speed instead

	
	//Slide forward
	// set CharMoveComp settings
	InitialGroundFrictionLevel = OwnerCharacterMovementComponent->GroundFriction;
	InitialBrakingFrictionLevel = OwnerCharacterMovementComponent->BrakingFriction;
	InitialBreakingFrictionFactor = OwnerCharacterMovementComponent->BrakingFrictionFactor;
	InitialDecelarationWalking = OwnerCharacterMovementComponent->BrakingDecelerationWalking;
	InitialValueOnSeperateBrakingFriction = OwnerCharacterMovementComponent->bUseSeparateBrakingFriction;
	
	OwnerCharacterMovementComponent->GroundFriction = SlideFrictionLevel;
	OwnerCharacterMovementComponent->BrakingFriction = SlideFrictionLevel;
	OwnerCharacterMovementComponent->BrakingFrictionFactor = SlideFrictionLevel;
	OwnerCharacterMovementComponent->BrakingDecelerationWalking = SlideFrictionLevel;
	OwnerCharacterMovementComponent->bUseSeparateBrakingFriction = false;

	
	// calculate dir and launch
	FVector ForwardBack = OwnerPawn->GetActorForwardVector();
	FVector ResultMoveDir = (ForwardBack).GetSafeNormal();
	OwnerCharacterMovementComponent->Launch(ResultMoveDir * SlideStrength);
	
	
	return true;
}

bool USlideVaultState::StopSlide()
{
	OwnerCharacterMovementComponent->GroundFriction = InitialGroundFrictionLevel;
	OwnerCharacterMovementComponent->BrakingFriction = InitialBrakingFrictionLevel;
	OwnerCharacterMovementComponent->BrakingFrictionFactor = InitialBreakingFrictionFactor;
	OwnerCharacterMovementComponent->BrakingDecelerationWalking = InitialDecelarationWalking;
	OwnerCharacterMovementComponent->bUseSeparateBrakingFriction = InitialValueOnSeperateBrakingFriction;

	CustomCharMoveComp->OnSlideVaultExecuted();

	return true;
}

bool USlideVaultState::TryWalkTransition(FMovementContext Context)
{
	FHitResult Hit = LookTraceSubsystem->GetHitResultFromPawnDownSphereTrace(
		OwnerPawn, 1000, 10, VaultTraceChannel);

	if (!Hit.GetActor())
	{
		FSM->SetState(UWalkState::StaticClass(), Context);
		return true;
	}
	if (!Hit.GetActor()->ActorHasTag(VaultableTagName))
	{
		FSM->SetState(UWalkState::StaticClass(), Context);
		return true;
	}

	return false;
}
