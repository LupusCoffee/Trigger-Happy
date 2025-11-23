#pragma once

#include "CoreMinimal.h"
#include "GP4Prototype/Public/Core/Data/Structs/MovementContext.h"
#include "UObject/Object.h"
#include "BaseMoveState.generated.h"


UCLASS()
class GP4PROTOTYPE_API UBaseMoveState : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual bool Enter(FMovementContext Context);
	
	UFUNCTION(BlueprintCallable)
	virtual bool Exit();
	
	UFUNCTION(BlueprintCallable)
	virtual void Tick(float DeltaTime, FMovementContext Context);


	UFUNCTION(BlueprintCallable)
	void SetFSM(UBaseMovementFiniteStateMachine* InputFSM);
	
	UFUNCTION(BlueprintCallable)
	UBaseMovementFiniteStateMachine* GetFSM();


protected:
	UPROPERTY()
	UBaseMovementFiniteStateMachine* FSM = nullptr;
};


//should be in a cpp file ugh
inline bool UBaseMoveState::Enter(FMovementContext Context)
{
	return true;
}

inline bool UBaseMoveState::Exit()
{
	return true;
}

inline void UBaseMoveState::Tick(float DeltaTime, FMovementContext Context)
{
	
}

inline void UBaseMoveState::SetFSM(UBaseMovementFiniteStateMachine* InputFSM)
{
	FSM = InputFSM;
}

inline UBaseMovementFiniteStateMachine* UBaseMoveState::GetFSM()
{
	return FSM;
}
