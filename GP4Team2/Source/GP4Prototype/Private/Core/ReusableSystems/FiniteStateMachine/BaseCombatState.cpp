#include "Core/ReusableSystems/FiniteStateMachine/BaseCombatState.h"
#include "Core/Data/Structs/CombatContext.h"

bool UBaseCombatState::Enter(FCombatContext Context)
{
	return true;
}

bool UBaseCombatState::Exit()
{
	return true;
}

void UBaseCombatState::Tick(float DeltaTime, FCombatContext Context)
{
	
}

void UBaseCombatState::SetFSM(UBaseCombatFiniteStateMachine* InputFSM)
{
	FSM = InputFSM;
}

UBaseCombatFiniteStateMachine* UBaseCombatState::GetFSM()
{
	return FSM;
}