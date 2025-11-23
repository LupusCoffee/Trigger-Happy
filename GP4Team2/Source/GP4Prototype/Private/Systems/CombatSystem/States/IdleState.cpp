#include "Systems/CombatSystem/States/IdleState.h"

#include "Systems/CombatSystem/States/AbilityState.h"
#include "Systems/CombatSystem/States/FireState.h"
#include "Systems/CombatSystem/States/MeleeState.h"
#include "Systems/MovementSystem/States/WalkState.h"


void UIdleState::Tick(float DeltaTime, FCombatContext Context)
{
	Super::Tick(DeltaTime, Context);

	if (Context.bWantsToFire) FSM->SetState(UFireState::StaticClass(), Context);
	if (Context.bWantsToMelee) FSM->SetState(UMeleeState::StaticClass(), Context);
	if (Context.bWantsToAbility) FSM->SetState(UAbilityState::StaticClass(), Context);
}
