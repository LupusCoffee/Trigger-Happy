#pragma once

#include "CoreMinimal.h"
#include "GP4Prototype/Public/Core/ReusableSystems/FiniteStateMachine/BaseMoveState.h"
#include "Systems/MovementSystem/CustomCharacterMovementComponent.h"
#include "WalkState.generated.h"


struct FMovementContext;
class UCharacterMovementComponent;

UCLASS()
class GP4PROTOTYPE_API UWalkState : public UBaseMoveState
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void Init(float InitFallbackMoveSpeed);

	virtual bool Enter(FMovementContext Context) override;
	virtual void Tick(float DeltaTime, FMovementContext Context) override;
	virtual bool Exit() override;

protected:
	// methods
	UFUNCTION()
	void Move(FVector2f MoveInput);
	
	// variables
	UPROPERTY()
	float FallbackMoveSpeed;

	UPROPERTY()
	UCustomCharacterMovementComponent* CustomCharMoveComp;

	UPROPERTY()
	UCharacterMovementComponent* OwnerCharacterMovementComponent;
	
	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	UAttributeComponent* AttributeComponent = nullptr;
};