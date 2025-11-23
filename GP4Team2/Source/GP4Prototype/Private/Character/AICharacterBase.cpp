// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AICharacterBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Systems/AISpawningSystem/AISpawnBrain.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "TimerManager.h"


// Sets default values
AAICharacterBase::AAICharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bStationaryPatrol = false;
	bStationaryAttack = false;
	bHasBeenInjured = false;
	bIsAggroed = false;
	PatrolRoute = nullptr;
	AttrComp = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

}

// Called when the game starts or when spawned
void AAICharacterBase::BeginPlay()
{
	Super::BeginPlay();
	if (PatrolRoute)
	{
		UE_LOG(LogTemp, Log, TEXT("%s is assigned to patrol route: %s"),
			*GetName(), *PatrolRoute->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no patrol route assigned!"), *GetName());
	}
	if (HealthComp)
	{
		HealthComp->OnDeathAsEnemy.AddDynamic(this, &AAICharacterBase::OnAIDeath);
	}
}

// Called every frame
void AAICharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAICharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAICharacterBase::InitializeAICharacterBase()
{
	if (!HealthComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Health is not Init"));
		return;
	};

	if (!AttrComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Attribute is not Init"));
		return;
	}
	if (HealthComp && AttrComp) {UE_LOG(LogTemp, Warning, TEXT("Both are Init"));}
	if (HealthComp) {HealthComp->InitHealth();}

	bHandledDeath = false;
}

void AAICharacterBase::OnAIDeath(EGameDamageType LastDamageTaken)
{
	// Guard against re-entrancy or duplicate handling
	if (bHandledDeath)
	{
		return;
	}
	bHandledDeath = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetAvoidanceEnabled(false);
		MoveComp->AvoidanceWeight = 0.0f;
	}
	if (SpawnBrain)
	{
		SpawnBrain->HandleAIDeath(this);
	}

	// Defer unbinding until after we exit the broadcast frame to avoid delegate access violations.
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (HealthComp)
		{
			HealthComp->OnDeathAsEnemy.RemoveDynamic(this, &AAICharacterBase::OnAIDeath);
		}
	}));
}
