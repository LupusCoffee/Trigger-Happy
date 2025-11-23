#include "GP4Prototype/Public/Systems/MovementSystem/States/SprintState.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "Components/CapsuleComponent.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseMovementFiniteStateMachine.h"
#include "Core/Subsystems/LookTraceSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Systems/MovementSystem/MovementFiniteStateMachine.h"
#include "Systems/MovementSystem/States/CrouchState.h"
#include "Systems/MovementSystem/States/DashState.h"
#include "Systems/MovementSystem/States/RegularVaultState.h"
#include "Systems/MovementSystem/States/SlideState.h"
#include "Systems/MovementSystem/States/SlideVaultState.h"
#include "Systems/MovementSystem/States/WalkState.h"

void USprintState::Init(float InitFallbackMoveSpeed, float InitVaultTraceLength, float InitVaultTraceRadius, ECollisionChannel InitVaultTraceChannel,
					    float InitVaultHeightRequirement, FName InitRegularVaultableTagName, FName InitSlideVaultableTagName)
{
	// inital setting
	FallbackMoveSpeed = InitFallbackMoveSpeed;
	VaultTraceLength = InitVaultTraceLength;
	VaultTraceRadius = InitVaultTraceRadius;
	VaultTraceChannel = InitVaultTraceChannel;
	VaultHeightRequirement = InitVaultHeightRequirement;
	RegularVaultableTagName = InitRegularVaultableTagName;
	SlideVaultableTagName = InitSlideVaultableTagName;

	
	// MovementFSM, OwnerCharacterMovementComponent, OwnerPawn
	UMovementFiniteStateMachine* MovementFSM = Cast<UMovementFiniteStateMachine>(FSM);
	if (!MovementFSM) return;

	CustomCharMoveComp = MovementFSM->GetOwnerCustomCharacterMovementComponent();

	if (AActor* OwnerActor = MovementFSM->GetOwnerActor())
	{
		OwnerCharacterMovementComponent = OwnerActor->GetComponentByClass<UCharacterMovementComponent>();
	}
	
	OwnerPawn = MovementFSM->GetOwnerPawn();
	
	if (OwnerPawn) AttributeComponent = OwnerPawn->GetComponentByClass<UAttributeComponent>();

	AActor* OwnerActor = MovementFSM->GetOwnerActor();
	if (OwnerActor) OwnerCapsuleComponent = OwnerActor->GetComponentByClass<UCapsuleComponent>();
	

	// set LookTraceSubsystem
	if (!OwnerPawn) return;
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	
	LookTraceSubsystem = LocalPlayer->GetSubsystem<ULookTraceSubsystem>();
}

bool USprintState::Enter(FMovementContext Context)
{
	//set character movement component max speed, acceleration, etc.
	
	float MoveSpeed = FallbackMoveSpeed;
	if (AttributeComponent) MoveSpeed = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Movement_SprintSpeed);
	
	if (OwnerCharacterMovementComponent) OwnerCharacterMovementComponent->MaxWalkSpeed = MoveSpeed;

	CustomCharMoveComp->OnSprintStart.Broadcast();
	
	return true;
}

void USprintState::Tick(float DeltaTime, FMovementContext Context)
{
	Super::Tick(DeltaTime, Context);
	
	Move(Context.MoveInput);

	if (TryVaultTransition(Context)) return; //maybe every 0.05 sec instead?
	if (!Context.bWantsToSprint) FSM->SetState(UWalkState::StaticClass(), Context);
	if (Context.bWantsToCrouch && !OwnerCharacterMovementComponent->IsFalling()) FSM->SetState(USlideState::StaticClass(), Context);
	if (Context.bWantsToDash) FSM->SetState(UDashState::StaticClass(), Context);
}

bool USprintState::Exit()
{
	CustomCharMoveComp->OnSprintFinish.Broadcast();
	
	return Super::Exit();
}

void USprintState::Move(FVector2f MoveInput)
{
	if (!OwnerPawn) return;
	
	FVector ForwardBack = OwnerPawn->GetActorForwardVector() * MoveInput.Y;
	FVector LeftRight = OwnerPawn->GetActorRightVector() * MoveInput.X;
	FVector ResultMoveDir = (ForwardBack+LeftRight).GetSafeNormal();
	
	OwnerPawn->AddMovementInput(ResultMoveDir);
}

bool USprintState::TryVaultTransition(FMovementContext Context)
{
	FHitResult Hit = LookTraceSubsystem->GetHitResultFromPawnForwardSphereTrace(
		OwnerPawn, VaultTraceLength, VaultTraceRadius, VaultTraceChannel);

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
