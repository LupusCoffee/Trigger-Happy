#include "Systems/CombatSystem/States/FireState.h"

#include "Core/Data/Enums/GameDamageType.h"
#include "Core/Data/Interfaces/Damageable.h"
#include "Systems/CombatSystem/CombatFiniteStateMachine.h"
#include "Systems/CombatSystem/States/AbilityState.h"
#include "Systems/CombatSystem/States/IdleState.h"
#include "Systems/CombatSystem/States/MeleeState.h"
#include "UObject/FastReferenceCollector.h"


void UFireState::Init(float InitDamage, float InitFireRate, float InitFireTraceLength, float InitFireTraceRadius,
					  USkeletalMeshComponent* InitCharArms, UAnimMontage* InitGunFireMontage)
{
	// initial stuff
	GunDamage = InitDamage;
	FireRate = InitFireRate;
	FireTraceLength = InitFireTraceLength;
	FireTraceRadius = InitFireTraceRadius;
	
	CharArms = InitCharArms;
	GunFireMontage = InitGunFireMontage;


	// combat fsm
	CombatFSM = Cast<UCombatFiniteStateMachine>(FSM);
	if (!CombatFSM) return;

	
	// set LookTraceSubsystem
	APawn* OwnerPawn = CombatFSM->GetOwnerPawn();
	if (!OwnerPawn) return;
	
	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	
	LookTraceSubsystem = LocalPlayer->GetSubsystem<ULookTraceSubsystem>();


	// set controller
	Controller = OwnerPawn->GetController();

	
	// set combat comp
	AActor* OwnerActor = CombatFSM->GetOwnerActor();
	if (!OwnerActor) return;
	
	CombatComponent = OwnerActor->GetComponentByClass<UCombatComponent>();


	// set anim instance
	if (!CharArms) return;
	AnimInstance = CharArms->GetAnimInstance();
}

bool UFireState::Enter(FCombatContext Context)
{
	if (Context.bFireChargeFullyDepleted) return false;
	
	FireTimer = FireRate;
	
	return true;
}

void UFireState::Tick(float DeltaTime, FCombatContext Context)
{
	Super::Tick(DeltaTime, Context);

	if (Context.bFireChargeFullyDepleted)
	{
		FSM->SetState(UIdleState::StaticClass(), Context);
		return;
	}
	
	if (Context.bHasNoFireCooldown) Fire();
	
	if (!Context.bWantsToFire) FSM->SetState(UIdleState::StaticClass(), Context);
	if (Context.bWantsToMelee) FSM->SetState(UMeleeState::StaticClass(), Context);
	if (Context.bWantsToAbility) FSM->SetState(UAbilityState::StaticClass(), Context);
}

void UFireState::Fire()
{
	// null checks
	if (!LookTraceSubsystem) return;
	if (!Controller) return;
	if (!CombatComponent) return;

	
	// fire event
	CombatComponent->OnFireExecuted();
	CombatComponent->Fire();
	CombatComponent->OnFire.Broadcast();
	

	// anim
	if (AnimInstance && GunFireMontage)
	{
		//float PlayRate = GunFireMontage->GetPlayLength() / FireRate;
		AnimInstance->Montage_Play(GunFireMontage, 2, EMontagePlayReturnType::MontageLength, 0, false);
	}
}
