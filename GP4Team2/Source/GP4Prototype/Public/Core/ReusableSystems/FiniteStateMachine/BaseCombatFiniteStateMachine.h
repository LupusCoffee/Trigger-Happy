#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BaseCombatFiniteStateMachine.generated.h"
struct FCombatContext;
class UBaseCombatState;

UCLASS()
class GP4PROTOTYPE_API UBaseCombatFiniteStateMachine : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual bool SetupStates(TArray<UBaseCombatState*> StatesToUse);

	UFUNCTION(BlueprintCallable)
	virtual bool SetState(UClass* StateClass, FCombatContext Context);

	UFUNCTION()
	virtual void Tick(float DeltaTime, FCombatContext Context);
	
protected:
	UPROPERTY()
	TMap<TSubclassOf<UBaseCombatState>, TObjectPtr<UBaseCombatState>> AvailableStates;
	
	UPROPERTY()
	TObjectPtr<UBaseCombatState> CurrentState;
};
