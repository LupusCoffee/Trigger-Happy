#include "GP4Prototype/Public/Core/ReusableSystems/FiniteStateMachine/BaseMovementFiniteStateMachine.h"
#include "GP4Prototype/Public/Core/ReusableSystems/FiniteStateMachine/BaseMoveState.h"

bool UBaseMovementFiniteStateMachine::SetupStates(TArray<UBaseMoveState*> StatesToUse)
{
	for (auto StateToUse : StatesToUse)
	{
		StateToUse->SetFSM(this);
		AvailableStates.FindOrAdd(StateToUse->GetClass(), StateToUse);
	}
	
	return true;
}

bool UBaseMovementFiniteStateMachine::SetState(UClass* StateClass, FMovementContext Context)
{
	// get state & null checks
	UBaseMoveState* State = *AvailableStates.Find(StateClass);
	if (!IsValid(State)) return false;
	if (CurrentState == State) return false;

	// exit
	if (IsValid(CurrentState))
	{
		bool StateExitStatus = CurrentState->Exit();
		if (!StateExitStatus) return false;
	}

	// enter
	bool StateEnterStatus = State->Enter(Context);
	if (!StateEnterStatus) return false;
	CurrentState = State;
	
	return true;
}

void UBaseMovementFiniteStateMachine::Tick(float DeltaTime, FMovementContext Context)
{
	if (IsValid(CurrentState))
		CurrentState->Tick(DeltaTime, Context);
}

bool UBaseMovementFiniteStateMachine::CurrentStateIs(UClass* StateClass)
{
	if (CurrentState.GetClass() == StateClass) return true;
	return false;
}
