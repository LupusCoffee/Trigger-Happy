#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseCombatState.h"
#include "Systems/CombatSystem/CombatFiniteStateMachine.h"
#include "Systems/CombatSystem/Components/AbilityComponent.h"
#include "AbilityState.generated.h"


UCLASS()
class GP4PROTOTYPE_API UAbilityState : public UBaseCombatState
{
	GENERATED_BODY()

public:
	void Init();
	virtual bool Enter(FCombatContext Context) override;
	virtual void Tick(float DeltaTime, FCombatContext Context) override;
	virtual bool Exit() override;

protected:
	// variables --> static values
	UPROPERTY()
	float Something;

	// variables --> components
	UPROPERTY()
	UCombatFiniteStateMachine* CombatFSM;

	UPROPERTY()
	AActor* OwnerActor;
	
	UPROPERTY()
	UAbilityComponent* AbilityComponent;

	UPROPERTY()
	UGameplayAbilityObject* EquippedAbility;
};
