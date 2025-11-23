#include "Core/ReusableSystems/FiniteStateMachine/BaseCombatFiniteStateMachine.h"
#include "Core/Data/Structs/CombatContext.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseCombatState.h"

bool UBaseCombatFiniteStateMachine::SetupStates(TArray<UBaseCombatState*> StatesToUse)
{
	for (auto StateToUse : StatesToUse)
	{
		StateToUse->SetFSM(this);
		AvailableStates.FindOrAdd(StateToUse->GetClass(), StateToUse);
	}
	
	return true;
}

bool UBaseCombatFiniteStateMachine::SetState(UClass* StateClass, FCombatContext Context)
{
	// get state & null checks
	UBaseCombatState* State = *AvailableStates.Find(StateClass);
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

void UBaseCombatFiniteStateMachine::Tick(float DeltaTime, FCombatContext Context)
{
	if (IsValid(CurrentState))
		CurrentState->Tick(DeltaTime, Context);
}
