#include "Systems/CombatSystem/Abilities/AbilityResurrect.h"
#include "Systems/CombatSystem/Components/AbilityComponent.h"
#include "Systems/CombatSystem/Components/HealthComponent.h"

class UHealthComponent;

void UAbilityResurrect::Init_Implementation(UAbilityComponent* _MyAbilityComponent)
{
	Super::Init_Implementation(_MyAbilityComponent);

	if (!MyAbilityComponent) return;
	
	AActor* Owner = MyAbilityComponent->GetOwner();
	if (!Owner) return;
	
	UHealthComponent* HealthComponent = Owner->GetComponentByClass<UHealthComponent>();
	if (!HealthComponent) return;

	HealthComponent->UnlockResurrect(PercentageOfMaxHealthToReplenish);
}
