#include "Systems/CombatSystem/States/AbilityState.h"

#include "Systems/CombatSystem/States/IdleState.h"

void UAbilityState::Init()
{
	// set combat fsm
	CombatFSM = Cast<UCombatFiniteStateMachine>(FSM);
	if (!CombatFSM) return;

	
	// set owner actor
	OwnerActor = CombatFSM->GetOwnerActor();
	if (!OwnerActor) return;


	// set ability component
	AbilityComponent = OwnerActor->GetComponentByClass<UAbilityComponent>();
	if (!AbilityComponent) return;
}

bool UAbilityState::Enter(FCombatContext Context)
{
	if (!AbilityComponent) return false;
	
	EquippedAbility = AbilityComponent->GetCurrentCombatAbility();
	if (!EquippedAbility) return false;

	bool CanBeUsed = EquippedAbility->StartUsing();
	if (!CanBeUsed) return false;
	
	return true;
}

void UAbilityState::Tick(float DeltaTime, FCombatContext Context)
{
	Super::Tick(DeltaTime, Context);

	EquippedAbility->TickUse(DeltaTime);

	if (!Context.bWantsToAbility && EquippedAbility->MustHoldDownInputThroughout)
	{
		FSM->SetState(UIdleState::StaticClass(), Context);
		return;
	}
	if (EquippedAbility->IsFinished()) FSM->SetState(UIdleState::StaticClass(), Context);
}

bool UAbilityState::Exit()
{
	if (!AbilityComponent) return false;
	
	if (!EquippedAbility) return false;

	EquippedAbility->StopUsing();	//hmmm this is done twice via ability component as well --> problem?

	EquippedAbility = nullptr;
	
	return true;
}
