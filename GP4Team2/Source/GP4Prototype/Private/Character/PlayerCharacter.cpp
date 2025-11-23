#include "Character/PlayerCharacter.h"

#include "Systems/CombatSystem/CombatComponent.h"
#include "Systems/CombatSystem/Components/AbilityComponent.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("CPP_AttributeComponent"));
	// UpgradeManager = CreateDefaultSubobject<UUpgradeManagerComponent>(TEXT("CPP_UpgradeManagerComponent"));
	// HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("CPP_HealthComponent"));
	// AbilityComponent = CreateDefaultSubobject<UAbilityComponent>(TEXT("CPP_AbilityComponent"));
	// CombatComponent  = CreateDefaultSubobject<UCombatComponent>(TEXT("CPP_CombatComponent"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

