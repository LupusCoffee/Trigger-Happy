#include "GP4Prototype/Public/Systems/CombatSystem/Components/HealthComponent.h"

#include "Core/Data/Enums/GameDamageType.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "Systems/AttributeSystem/AttributeTags.h"
#include "SimplifiedDebugMessage/Public/Debug.h"
#include "Systems/CombatSystem/CombatEventsSubsystem.h"


class UCombatEventsSubsystem;
//Setup / Core Overrides
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHealthComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	//Comps and such
	AActor* Owner = GetOwner();
	if (Owner) AttributeComponent = Owner->GetComponentByClass<UAttributeComponent>();
	
	CombatEventsSubsystem = GetWorld()->GetSubsystem<UCombatEventsSubsystem>();
	bIsPlayer = OwnerIsPlayerControlled();

	//Inits
	InitHealth();
}

bool UHealthComponent::OwnerIsPlayerControlled() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	if (Owner->IsA<APlayerController>()) return true;

	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn) return false;

	if (Pawn->IsPlayerControlled()) return true;
	return false;
}

void UHealthComponent::InitHealth()
{
	float MaxHealth = FallbackMaxHealth;
	if (AttributeComponent) MaxHealth =	AttributeComponent->GetAttributeValue(AttributeTags::Attribute_MaxHealth);

	CurrentHealth = MaxHealth;
}


//Tick
void UHealthComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickShieldRecharge(DeltaTime);
}


//Actual Uses
float UHealthComponent::Heal(float Value)
{
	if (bIsDead) return -1.f;

	float MaxHealth = FallbackMaxHealth;
	if (AttributeComponent) MaxHealth =	AttributeComponent->GetAttributeValue(AttributeTags::Attribute_MaxHealth);

	CurrentHealth += Value;
	if (CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
	if (CurrentHealth < 0.f)
	{
		CurrentHealth = 0.f;
	}
	OnHeal.Broadcast(Value);
	return CurrentHealth;
}

void UHealthComponent::TakeHealthDamage(EGameDamageType DamageType, float DamageValue, float DamageMultiplier)
{
	if (bIsDead) return;

	TryTakeShieldDamage(DamageType, DamageValue, DamageMultiplier);
	
	CurrentHealth -= DamageValue * DamageMultiplier;
	if (CurrentHealth <= 0) Die(DamageType);

	OnHealthDamage.Broadcast(DamageType, DamageValue);
}

void UHealthComponent::TryTakeShieldDamage(EGameDamageType DamageType, float& DamageValue, float DamageMultiplier)
{
	if (!bShieldUnlocked) return;

	float TempValue = CurrentShieldValue - (DamageValue*DamageMultiplier);
	float DamageDealtToShield = 0;
	if (TempValue < 0) DamageDealtToShield = CurrentShieldValue;
	if (TempValue >= 0) DamageDealtToShield = (DamageValue*DamageMultiplier);
	
	CurrentShieldValue -= DamageDealtToShield;
	if (CurrentShieldValue < 0) CurrentShieldValue = 0;

	DamageValue -= DamageDealtToShield;
	if (DamageValue < 0) DamageValue = 0;

	if (DamageDealtToShield > 0) OnShieldDamage.Broadcast(DamageType, DamageDealtToShield);

	CurrentTimeBeforeShieldRecharge = 0;
}

void UHealthComponent::TickShieldRecharge(float DeltaTime)
{
	if (!bShieldUnlocked) return;
	
	if (CurrentShieldValue >= MaxShieldValue) return;

	if (CurrentTimeBeforeShieldRecharge < TimeBeforeShieldRecharge)
	{
		CurrentTimeBeforeShieldRecharge += DeltaTime;
	}
	else
	{
		CurrentShieldValue += DeltaTime * ShieldRechargeRate;
		if (CurrentShieldValue > MaxShieldValue) CurrentShieldValue = MaxShieldValue;
	}
}

void UHealthComponent::Die(EGameDamageType LastDamageTaken)
{
	if (bResurrectUnlocked && !bResurrectHasBeenUsed)
	{
		float MaxHealth = FallbackMaxHealth;
		if (AttributeComponent) MaxHealth =	AttributeComponent->GetAttributeValue(AttributeTags::Attribute_MaxHealth);
		
		CurrentHealth = MaxHealth * PercentageOfMaxHealthToReplenish;

		bResurrectHasBeenUsed = true;

		OnResurrect.Broadcast();
		
		return;
	}
	
	if (bIsDead) return;
	
	bIsDead = true;
	
	if (bIsPlayer) PlayerDeath(LastDamageTaken);
	else EnemyDeath(LastDamageTaken);
}

void UHealthComponent::PlayerDeath(EGameDamageType LastDamageTaken)
{
	GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, "Player Death");

	//disable player movement		--> set movement comp fsm to dead state
	//disable shooting and such		--> set combat comp fsm to dead state
	//disable player controller? necessary at all? prolly not

	OnDeathAsPlayer.Broadcast(LastDamageTaken);
}

void UHealthComponent::EnemyDeath(EGameDamageType LastDamageTaken)
{
	GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, "Enemy Death");

	switch (LastDamageTaken)
	{
		case EGameDamageType::Gun:
		CombatEventsSubsystem->OnEnemyKilled.Broadcast();
		CombatEventsSubsystem->OnEnemyKilledWithGun.Broadcast();
		break;

		case EGameDamageType::Melee:
		CombatEventsSubsystem->OnEnemyKilled.Broadcast();
		CombatEventsSubsystem->OnEnemyKilledWithMelee.Broadcast();
		break;

		case EGameDamageType::Explosion:
		CombatEventsSubsystem->OnEnemyKilled.Broadcast();
		CombatEventsSubsystem->OnEnemyKilledWithExplosion.Broadcast();
		break;
	}
	
	OnDeathAsEnemy.Broadcast(LastDamageTaken);
}


//Setters
void UHealthComponent::UnlockShield(float InMaxShieldValue, float InTimeBeforeRecharge, float InRechargeRate)
{
	bShieldUnlocked = true;

	MaxShieldValue = InMaxShieldValue;
	CurrentShieldValue = InMaxShieldValue;
	TimeBeforeShieldRecharge = InTimeBeforeRecharge;
	ShieldRechargeRate = InRechargeRate;
}

void UHealthComponent::UnlockResurrect(float InPercentageOfMaxHealthToReplenish)
{
	bResurrectUnlocked = true;
	
	PercentageOfMaxHealthToReplenish = InPercentageOfMaxHealthToReplenish;
}
