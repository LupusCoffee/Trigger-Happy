#pragma once

#include "CoreMinimal.h"
#include "BaseCombatFiniteStateMachine.h"
#include "Core/Data/Structs/CombatContext.h"
#include "GP4Prototype/Public/Core/Data/Structs/MovementContext.h"
#include "UObject/Object.h"
#include "BaseCombatState.generated.h"

UCLASS()
class GP4PROTOTYPE_API UBaseCombatState : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual bool Enter(FCombatContext Context);
	
	UFUNCTION(BlueprintCallable)
	virtual bool Exit();
	
	UFUNCTION(BlueprintCallable)
	virtual void Tick(float DeltaTime, FCombatContext Context);


	UFUNCTION(BlueprintCallable)
	void SetFSM(UBaseCombatFiniteStateMachine* InputFSM);
	
	UFUNCTION(BlueprintCallable)
	UBaseCombatFiniteStateMachine* GetFSM();


protected:
	UPROPERTY()
	UBaseCombatFiniteStateMachine* FSM = nullptr;
};
