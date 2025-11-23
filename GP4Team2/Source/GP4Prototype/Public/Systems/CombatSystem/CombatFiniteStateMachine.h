#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseCombatFiniteStateMachine.h"
#include "CombatFiniteStateMachine.generated.h"


UCLASS()
class GP4PROTOTYPE_API UCombatFiniteStateMachine : public UBaseCombatFiniteStateMachine
{
	GENERATED_BODY()

public:
	UFUNCTION()
	bool Init(AActor* InputOwnerActor, APawn* InputOwnerPawn);

	UFUNCTION()
	AActor* GetOwnerActor();

	UFUNCTION()
	APawn* GetOwnerPawn();

	UFUNCTION(BlueprintCallable)
	virtual bool CurrentStateIs(UClass* StateClass);

protected:
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;
};
