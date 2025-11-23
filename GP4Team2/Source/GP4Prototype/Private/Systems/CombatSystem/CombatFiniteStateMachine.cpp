#include "Systems/CombatSystem/CombatFiniteStateMachine.h"

bool UCombatFiniteStateMachine::Init(AActor* InputOwnerActor, APawn* InputOwnerPawn)
{
	if (!InputOwnerActor) return false;

	OwnerActor = InputOwnerActor;
	OwnerPawn = InputOwnerPawn;
	
	return true;
}

AActor* UCombatFiniteStateMachine::GetOwnerActor()
{
	return OwnerActor;
}

APawn* UCombatFiniteStateMachine::GetOwnerPawn()
{
	return OwnerPawn;
}

bool UCombatFiniteStateMachine::CurrentStateIs(UClass* StateClass)
{
	if (CurrentState.GetClass() == StateClass) return true;
	return false;
}
