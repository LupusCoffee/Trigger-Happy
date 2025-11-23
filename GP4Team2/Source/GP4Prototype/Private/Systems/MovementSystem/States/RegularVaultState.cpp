#include "Systems/MovementSystem/States/RegularVaultState.h"
#include "Components/CapsuleComponent.h"
#include "Core/Subsystems/LookTraceSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Systems/MovementSystem/MovementFiniteStateMachine.h"
#include "Systems/MovementSystem/States/WalkState.h"
class ULookTraceSubsystem;

void URegularVaultState::Init()
{
	// owner pawn
	UMovementFiniteStateMachine* MovementFSM = Cast<UMovementFiniteStateMachine>(FSM);
	if (!MovementFSM) return;
	
	OwnerPawn = MovementFSM->GetOwnerPawn();

	CustomCharMoveComp = MovementFSM->GetOwnerCustomCharacterMovementComponent();
}

bool URegularVaultState::Enter(FMovementContext Context)
{
	if (!Context.bCanVault) return false;

	OwnerPawn->SetActorLocation(Context.VaultStartLocation); //lerp to this position with an assignable speed instead

	CustomCharMoveComp->OnRegularVaultStart.Broadcast();
	
	return true;
}

void URegularVaultState::Tick(float DeltaTime, FMovementContext Context)
{
	Super::Tick(DeltaTime, Context);

	FSM->SetState(UWalkState::StaticClass(), Context);
}

bool URegularVaultState::Exit()
{
	CustomCharMoveComp->OnRegularVaultFinish.Broadcast();
	
	return true;
}
