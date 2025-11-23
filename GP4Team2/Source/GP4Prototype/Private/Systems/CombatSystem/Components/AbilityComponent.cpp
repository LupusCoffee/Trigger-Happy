#include "Systems/CombatSystem/Components/AbilityComponent.h"

#include "Kismet/GameplayStatics.h"

// setup
UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	WantsToSupportAbility = false;
}

void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (StartingCombatAbility) SetCurrentCombatAbility(StartingCombatAbility);
	if (StartingSupportAbility) SetCurrentSupportAbility(StartingSupportAbility);

	for (auto StartingPassiveAbility : StartingPassiveAbilities)
		AddPassiveAbility(StartingPassiveAbility);
}


// tick
void UAbilityComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// tick use --> AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH AAAHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
	if (WantsToSupportAbility) TickAbilityUse(CurrentSupportAbility, DeltaTime);
	else
	{
		if (SupportAbilityUsed)
		{
			CurrentSupportAbility->StopUsing();

			SupportAbilityUsed = false;
		}
	}
	
	TickCurrentPassiveAbilitiesUse(DeltaTime);
	
	// tick cooldowns
	TickAbilityCooldown(CurrentCombatAbility, DeltaTime);
	TickAbilityCooldown(CurrentSupportAbility, DeltaTime);
	TickCurrentPassiveAbilitiesCooldown(DeltaTime);
}

void UAbilityComponent::StartAbility(UGameplayAbilityObject* Ability)
{
	if (!Ability) return;
	if (!Ability->CanBeUsed()) return;
	Ability->StartUsing();
	SupportAbilityUsed = true;
}

void UAbilityComponent::TickAbilityUse(UGameplayAbilityObject* Ability, float DeltaTime)
{
	if (!Ability) return;
	if (!Ability->ShouldTick()) return;
	
	Ability->TickUse(DeltaTime);
	if (Ability->IsFinished()) Ability->StopUsing();
}

void UAbilityComponent::TickCurrentPassiveAbilitiesUse(float DeltaTime)
{
	for (auto PassiveAbility : CurrentPassiveAbilities)
		TickAbilityUse(PassiveAbility, DeltaTime);
}

void UAbilityComponent::TickAbilityCooldown(UGameplayAbilityObject* Ability, float DeltaTime)
{
	if (!Ability) return;
	if (Ability->CanBeUsed()) return;
	
	Ability->TickCooldown(DeltaTime);
}

void UAbilityComponent::TickCurrentPassiveAbilitiesCooldown(float DeltaTime)
{
	for (auto PassiveAbility : CurrentPassiveAbilities)
		TickAbilityCooldown(PassiveAbility, DeltaTime);
}


// setters / adders
void UAbilityComponent::SetCurrentCombatAbility(UGameplayAbilityObject* Ability)
{
	if (!Ability) return;

	CurrentCombatAbility = Ability;
	
	CurrentCombatAbility->Init(this);
}

void UAbilityComponent::SetCurrentSupportAbility(UGameplayAbilityObject* Ability)
{
	if (!Ability) return;

	CurrentSupportAbility = Ability;
	
	CurrentSupportAbility->Init(this);
}

void UAbilityComponent::AddPassiveAbility(UGameplayAbilityObject* Ability)
{
	if (!Ability) return;

	CurrentPassiveAbilities.Add(Ability);

	Ability->Init(this); // initialize the ability, if not just straight up start it
}


// getters
UGameplayAbilityObject* UAbilityComponent::GetCurrentCombatAbility()
{
	return CurrentCombatAbility;
}

UGameplayAbilityObject* UAbilityComponent::GetCurrentSupportAbility()
{
	return CurrentSupportAbility;
}



//TEMPORARY AHHHHHHH
void UAbilityComponent::Ability_BoxTraceMulti(const UObject* WorldContextObject, FVector Start, FVector End,
                                              FVector HalfSize, FRotator Orientation, TEnumAsByte<ETraceTypeQuery> TraceChannel, bool bTraceComplex,
                                              const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, TArray<FHitResult>& OutHits,
                                              bool bIgnoreSelf)
{
	UKismetSystemLibrary::BoxTraceMulti(
			WorldContextObject,
			Start,
			End,
			HalfSize,
			Orientation,
			TraceChannel,
			bTraceComplex,
			ActorsToIgnore,
			DrawDebugType,
			OutHits,
			bIgnoreSelf
		);
}

AActor* UAbilityComponent::SpawnActorForAbility(TSubclassOf<AActor> Class, const FTransform& Xform,
	ESpawnActorCollisionHandlingMethod Handling)
{
	FActorSpawnParameters P;
	P.Owner = GetOwner();
	P.SpawnCollisionHandlingOverride = Handling;
	return GetWorld()->SpawnActor<AActor>(Class, Xform, P);
}

void UAbilityComponent::PlaySound2D(USoundBase* Sound, UAudioComponent*& OutAudioComp)
{
	if (Sound && GetWorld())
	{
		UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(this, Sound);
		OutAudioComp = AudioComp;
	}
}
