#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BaseMovementFiniteStateMachine.generated.h"
class UBaseMoveState;


UCLASS()
class GP4PROTOTYPE_API UBaseMovementFiniteStateMachine : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual bool SetupStates(TArray<UBaseMoveState*> StatesToUse);

	UFUNCTION(BlueprintCallable)
	virtual bool SetState(UClass* StateClass, FMovementContext Context);

	UFUNCTION()
	virtual void Tick(float DeltaTime, FMovementContext Context);

	UFUNCTION(BlueprintCallable)
	virtual bool CurrentStateIs(UClass* StateClass);
	
protected:
	UPROPERTY()
	TMap<TSubclassOf<UBaseMoveState>, TObjectPtr<UBaseMoveState>> AvailableStates;
	
	UPROPERTY()
	TObjectPtr<UBaseMoveState> CurrentState;
};
