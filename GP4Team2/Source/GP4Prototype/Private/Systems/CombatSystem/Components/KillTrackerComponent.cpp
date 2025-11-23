#include "Systems/CombatSystem/Components/KillTrackerComponent.h"

#include <Systems/AttributeSystem/AttributeTags.h>

#include "Systems/AttributeSystem/AttributeComponent.h"


// setup / tick
UKillTrackerComponent::UKillTrackerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKillTrackerComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner) AttributeComponent = Owner->GetComponentByClass<UAttributeComponent>();
}

void UKillTrackerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastKill += DeltaTime;
	if (TimeSinceLastKill > MaxTimeBetweenKills) CurrentKillStreak = 0;
}


// setters / adders
void UKillTrackerComponent::AddKill()
{
	CurrentTotalKills++;
	CurrentKillStreak++;
	TimeSinceLastKill = 0;

	int KillStreakForOneShotBullet = FallbackKillStreakForOneShotBullet;
	if (AttributeComponent) KillStreakForOneShotBullet = AttributeComponent->GetAttributeValue(AttributeTags::Attribute_Killstreak_ExplosiveRounds);
	
	if (KillStreakForOneShotBullet < 1) KillStreakForOneShotBullet = 1;
	
	if (CurrentKillStreak % KillStreakForOneShotBullet == 0)
	{
		OnKillInARowThresholdForOneShotBulletAchieved.Broadcast(OneShotBullet);
	}

	if (CurrentKillStreak % KillStreakForLifeSteal == 0)
	{
		OnKillInARowThresholdForLifeStealAchieved.Broadcast(HealthToRegainUponLifeSteal);
	}
}


// getters
bool UKillTrackerComponent::IsKillStreakActive()
{
	return TimeSinceLastKill < MaxTimeBetweenKills;
}
