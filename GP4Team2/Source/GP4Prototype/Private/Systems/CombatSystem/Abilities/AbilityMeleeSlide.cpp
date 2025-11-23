#include "Systems/CombatSystem/Abilities/AbilityMeleeSlide.h"

#include "Systems/CombatSystem/Components/AbilityComponent.h"
#include "Systems/MovementSystem/CustomCharacterMovementComponent.h"

class UCustomCharacterMovementComponent;

void UAbilityMeleeSlide::Init_Implementation(UAbilityComponent* _MyAbilityComponent)
{
	Super::Init_Implementation(_MyAbilityComponent);

	Super::Init_Implementation(_MyAbilityComponent);

	if (!MyAbilityComponent) return;
	
	AActor* Owner = MyAbilityComponent->GetOwner();
	if (!Owner) return;
	
	UCustomCharacterMovementComponent* CustomMoveCharComp = Owner->GetComponentByClass<UCustomCharacterMovementComponent>();
	if (!CustomMoveCharComp) return;

	CustomMoveCharComp->UnlockMeleeSlide();
}
