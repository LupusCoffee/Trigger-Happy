#include "GP4Prototype/Public/Systems/MovementSystem/MovementFiniteStateMachine.h"

bool UMovementFiniteStateMachine::Init(AActor* InputOwnerActor, APawn* InputOwnerPawn, UCustomCharacterMovementComponent* InputMovementComponent)
{
	if (!InputOwnerActor) return false;
	if (!InputOwnerPawn) return false;

	OwnerActor = InputOwnerActor;
	OwnerPawn = InputOwnerPawn;
	OwnerCustomCharacterMovementComponent = InputMovementComponent;
	
	return true;
}

AActor* UMovementFiniteStateMachine::GetOwnerActor()
{
	return OwnerActor;
}

APawn* UMovementFiniteStateMachine::GetOwnerPawn()
{
	return OwnerPawn;
}

UCustomCharacterMovementComponent* UMovementFiniteStateMachine::GetOwnerCustomCharacterMovementComponent()
{
	return OwnerCustomCharacterMovementComponent;
}
