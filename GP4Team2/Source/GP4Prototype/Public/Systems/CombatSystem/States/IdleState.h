#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseCombatState.h"
#include "IdleState.generated.h"


UCLASS()
class GP4PROTOTYPE_API UIdleState : public UBaseCombatState
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime, FCombatContext Context) override;
};
