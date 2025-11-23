#pragma once

#include "CoreMinimal.h"
#include "Core/ReusableSystems/FiniteStateMachine/BaseMoveState.h"
#include "RegularVaultState.generated.h"

class ULookTraceSubsystem;
class UCapsuleComponent;
class APawn;
class UCustomCharacterMovementComponent;
class UCharacterMovementComponent;
struct FMovementContext;


UCLASS()
class GP4PROTOTYPE_API URegularVaultState : public UBaseMoveState
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void Init();

	virtual bool Enter(FMovementContext Context) override;
	virtual void Tick(float DeltaTime, FMovementContext Context) override;
	virtual bool Exit() override;

protected:
	UPROPERTY()
	APawn* OwnerPawn;

	UPROPERTY()
	UCustomCharacterMovementComponent* CustomCharMoveComp;
};