#include "Systems/CombatSystem/Abilities/AbilitySlowMo.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "Kismet/GameplayStatics.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "Systems/CombatSystem/Components/AbilityComponent.h"

void UAbilitySlowMo::Init_Implementation(UAbilityComponent* _MyAbilityComponent)
{
	Super::Init_Implementation(_MyAbilityComponent);

	World = MyAbilityComponent->GetWorld();
	//set CharMoveComp?

	if (MyAbilityComponent)
	{
		AActor* Owner = MyAbilityComponent->GetOwner();
		if (Owner) AttributeComponent = Owner->GetComponentByClass<UAttributeComponent>();
	}

	float TimeDilDuration = FallbackTimeDilationDuration;
	if (AttributeComponent) TimeDilDuration = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_SlowMo_MaxDuration);

	float TimeDilStrength = FallbackTimeDilationStrength;
	if (AttributeComponent) TimeDilStrength = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_SlowMo_TimeDilation);
	
	AbilityDuration = TimeDilStrength * TimeDilDuration;
}

bool UAbilitySlowMo::StartUsing()
{
	float TimeDilDuration = FallbackTimeDilationDuration;
	if (AttributeComponent) TimeDilDuration = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_SlowMo_MaxDuration);

	float TimeDilStrength = FallbackTimeDilationStrength;
	if (AttributeComponent) TimeDilStrength = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_SlowMo_TimeDilation);
	
	AbilityDuration = TimeDilStrength * TimeDilDuration;

	
	float SlowMoCooldown = FallbackSlowMoCooldown;
	if (AttributeComponent) SlowMoCooldown = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_SlowMo_Cooldown);
	
	AbilityCooldown = SlowMoCooldown;
	
	
	UGameplayStatics::SetGlobalTimeDilation(World, TimeDilStrength);

	//set player speed during slow mo?
	
	return Super::StartUsing();
}

void UAbilitySlowMo::StopUsing()
{
	Super::StopUsing();

	UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);

	//set player speed back to normal?
}
