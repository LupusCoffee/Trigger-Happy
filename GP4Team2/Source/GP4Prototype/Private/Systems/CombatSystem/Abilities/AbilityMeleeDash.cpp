#include "Systems/CombatSystem/Abilities/AbilityMeleeDash.h"
#include "Systems/CombatSystem/Components/AbilityComponent.h"
#include "Systems/MovementSystem/CustomCharacterMovementComponent.h"

void UAbilityMeleeDash::Init_Implementation(UAbilityComponent* _MyAbilityComponent)
{
	Super::Init_Implementation(_MyAbilityComponent);

	if (!MyAbilityComponent) return;
	
	AActor* Owner = MyAbilityComponent->GetOwner();
	if (!Owner) return;
	
	UCustomCharacterMovementComponent* CustomMoveCharComp = Owner->GetComponentByClass<UCustomCharacterMovementComponent>();
	if (!CustomMoveCharComp) return;

	CustomMoveCharComp->UnlockMeleeDash();
}
