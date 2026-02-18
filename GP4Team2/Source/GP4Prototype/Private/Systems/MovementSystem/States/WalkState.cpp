#include "GP4Prototype/Public/Systems/MovementSystem/States/WalkState.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "GameFramework/CharacterMovementComponent.h"
#include "GP4Prototype/Public/Systems/MovementSystem/MovementFiniteStateMachine.h"
#include "Systems/MovementSystem/States/CrouchState.h"
#include "Systems/MovementSystem/States/DashState.h"
#include "Systems/MovementSystem/States/SprintState.h"

void UWalkState::Init(float InitFallbackMoveSpeed)
{
	FallbackMoveSpeed = InitFallbackMoveSpeed;

	UMovementFiniteStateMachine* MovementFSM = Cast<UMovementFiniteStateMachine>(FSM);
	if (!MovementFSM) return;
	
	CustomCharMoveComp = MovementFSM->GetOwnerCustomCharacterMovementComponent();
	
	if (AActor* OwnerActor = MovementFSM->GetOwnerActor())
		OwnerCharacterMovementComponent = OwnerActor->GetComponentByClass<UCharacterMovementComponent>();

	OwnerPawn = MovementFSM->GetOwnerPawn();

	if (OwnerPawn) AttributeComponent = OwnerPawn->GetComponentByClass<UAttributeComponent>();
}

bool UWalkState::Enter(FMovementContext Context)
{
	//set character movement component max speed, acceleration, etc.
	
	float MoveSpeed = FallbackMoveSpeed;
	if (AttributeComponent) MoveSpeed = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Movement_WalkSpeed);
	
	if (OwnerCharacterMovementComponent) OwnerCharacterMovementComponent->MaxWalkSpeed = MoveSpeed;

	CustomCharMoveComp->OnWalkStateEnter.Broadcast();
	
	return true;
}

void UWalkState::Tick(float DeltaTime, FMovementContext Context)
{
	Super::Tick(DeltaTime, Context);

	if (!OwnerPawn) return;
	
	Move(Context.MoveInput);

	if (Context.bWantsToSprint && OwnerPawn->GetVelocity().Size() > 0) FSM->SetState(USprintState::StaticClass(), Context);
	if (Context.bWantsToCrouch) FSM->SetState(UCrouchState::StaticClass(), Context);
	if (Context.bWantsToDash) FSM->SetState(UDashState::StaticClass(), Context);
}

bool UWalkState::Exit()
{
	CustomCharMoveComp->OnWalkStateExit;
	
	return Super::Exit();
}

void UWalkState::Move(FVector2f MoveInput)
{
	FVector ForwardBack = OwnerPawn->GetActorForwardVector() * MoveInput.Y;
	FVector LeftRight = OwnerPawn->GetActorRightVector() * MoveInput.X;
	FVector ResultMoveDir = (ForwardBack+LeftRight).GetSafeNormal();
	
	OwnerPawn->AddMovementInput(ResultMoveDir);
}
