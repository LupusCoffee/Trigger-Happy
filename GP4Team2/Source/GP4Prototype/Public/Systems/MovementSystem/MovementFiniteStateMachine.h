#pragma once

#include "CoreMinimal.h"
#include "GP4Prototype/Public/Core/ReusableSystems/FiniteStateMachine/BaseMovementFiniteStateMachine.h"
#include "MovementFiniteStateMachine.generated.h"


class UCustomCharacterMovementComponent;

UCLASS()
class GP4PROTOTYPE_API UMovementFiniteStateMachine : public UBaseMovementFiniteStateMachine
{
	GENERATED_BODY()

public:
	UFUNCTION()
	bool Init(AActor* InputOwnerActor, APawn* InputOwnerPawn, UCustomCharacterMovementComponent* InputMovementComponent);

	UFUNCTION()
	AActor* GetOwnerActor();
	
	UFUNCTION()
	APawn* GetOwnerPawn();

	UFUNCTION()
	UCustomCharacterMovementComponent* GetOwnerCustomCharacterMovementComponent();

protected:
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;
	
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;

	UPROPERTY()
	TObjectPtr<UCustomCharacterMovementComponent> OwnerCustomCharacterMovementComponent;
};
