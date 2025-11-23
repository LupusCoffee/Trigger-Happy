// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/Controllers/AIRangeController.h"


// Sets default values
AAIRangeController::AAIRangeController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Create Perception Component
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	// Sight Config
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 2000.f;
		SightConfig->LoseSightRadius = 2200.f;
		SightConfig->PeripheralVisionAngleDegrees = 60.f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		AIPerceptionComponent->ConfigureSense(*SightConfig);
	}

	// Hearing Config
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = 1500.f;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

		AIPerceptionComponent->ConfigureSense(*HearingConfig);
	}

	// Make sight the dominant sense
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

// Called when the game starts or when spawned
void AAIRangeController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAIRangeController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAIRangeController::OnPossess(APawn* PossessedPawn)
{
	Super::OnPossess(PossessedPawn);
	OwnedPossessedPawn = Cast<AAICharacterBase>(PossessedPawn);

}

