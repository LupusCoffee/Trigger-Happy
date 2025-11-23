// Fill out your copyright notice in the Description page of Project Settings.

#include "Systems/UpgradeSystem/UpgradeManagerComponent.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameModeBase.h"

#include "DataStructures/AttributeUpgradeDataStructs.h"
#include "Systems/UpgradeSystem/RarityData.h"
#include "Systems/UpgradeSystem/UpgradeCardData.h"
#include "Curves/CurveFloat.h"
#include "Engine/World.h"
#include "Templates/Function.h"
#include "Systems/CombatSystem/Abilities/GameplayAbilityObject.h"
#include "SimplifiedDebugMessage/Public/Debug.h"
#include "UObject/UnrealType.h"
#include "Engine/Engine.h"
#include "UObject/UObjectIterator.h"
#include "Systems/CombatSystem/Components/AbilityComponent.h"

// REMOVE: DataTable row helpers/converters
// static void GatherRowsFlexible(...);
// static void ConvertEntriesToRows(...)

// -- Subsystem public API --

void UUpgradeManagerComponent::RequestHighRarityBonusNextRoll(UObject* Context)
{
	if (!Context)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestHighRarityBonusNextRoll called with null Context"));
		return;
	}
	PendingHighRarityBonus.Add(Context);
	if (bLogUpgradeRolls)
	{
		UE_LOG(LogTemp, Log, TEXT("RequestHighRarityBonusNextRoll: Added bonus for Context %s"), *Context->GetName());
	}
}

bool UUpgradeManagerComponent::HasPendingHighRarityBonusFor(UObject* Context) const
{
	return Context && PendingHighRarityBonus.Contains(Context);
}

bool UUpgradeManagerComponent::ConsumePendingBonusFor(UObject* Context)
{
	if (!Context) return false;
	bool bRemoved = PendingHighRarityBonus.Remove(Context) > 0;
	if (bRemoved && bLogUpgradeRolls)
	{
		UE_LOG(LogTemp, Log, TEXT("ConsumePendingBonusFor: Consumed bonus for Context %s"), *Context->GetName());
	}
	return bRemoved;
}

// Convenience: request using AttributeComponent; resolves to same context logic as RollUpgrades
void UUpgradeManagerComponent::RequestHighRarityBonusNextRollForAttributes(UAttributeComponent* TargetAttributes)
{
	if (!TargetAttributes)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestHighRarityBonusNextRollForAttributes called with null TargetAttributes"));
		return;
	}
	UObject* OwnerCtx = TargetAttributes->GetOwner();
	UObject* ChosenCtx = OwnerCtx ? OwnerCtx : static_cast<UObject*>(TargetAttributes);
	RequestHighRarityBonusNextRoll(ChosenCtx);
}

int32 UUpgradeManagerComponent::GetCurrentFloor() const
{
	// Debug override takes precedence
	if (bUseDebugFloorOverride)
	{
		return FMath::Max(1, DebugFloorOverride);
	}
	// LevelManager actor has a method to get current floor called GetFloor()
	if (LevelManager)
	{
		if (UFunction* Func = LevelManager->FindFunction(FName("GetFloor")))
		{
			struct FReturnValue { int32 Floor; };
			FReturnValue ReturnValue{ -1 };
			LevelManager->ProcessEvent(Func, &ReturnValue);
			return ReturnValue.Floor;
		}
	}
	return -1; // Invalid floor
}

ERarity UUpgradeManagerComponent::RollRarity(int Difficulty)
{
	// Build base weights from RarityData
	struct FWeightedRarity { ERarity Rarity; float Weight; int32 Rank; };
	TArray<FWeightedRarity> Weights;
	Weights.Reserve(RarityDataList.Num());

	// Compute min/max sort order for rank normalization
	int32 MinSort = INT_MAX, MaxSort = INT_MIN;
	for (const URarityData* Data : RarityDataList)
	{
		if (!Data) continue;
		MinSort = FMath::Min(MinSort, Data->SortOrder);
		MaxSort = FMath::Max(MaxSort, Data->SortOrder);
	}
	const int32 SortRange = FMath::Max(1, MaxSort - MinSort);

	for (const URarityData* Data : RarityDataList)
	{
		if (!Data) continue;
		Weights.Add({ Data->Rarity, FMath::Max(0.f, Data->RollWeight), Data->SortOrder - MinSort });
	}

	if (Weights.Num() == 0)
	{
		return ERarity::Common;
	}

	// Optional difficulty bias curve (0..1)
	float BiasT = 0.f;
	if (RarityBiasCurve)
	{
		BiasT = FMath::Clamp(RarityBiasCurve->GetFloatValue((float)Difficulty), 0.f, 1.f);
	}
	if (BiasT > 0.f)
	{
		for (FWeightedRarity& WR : Weights)
		{
			const float Rank01 = (float)WR.Rank / (float)SortRange; // 0 = lowest, 1 = highest
			const float BiasScale = 1.f + BiasT * (0.5f + 2.5f * Rank01); // scale ~ [1..~4]
			WR.Weight *= BiasScale;
		}
	}

	float TotalWeight = 0.f;
	for (const FWeightedRarity& WR : Weights) { TotalWeight += WR.Weight; }
	if (TotalWeight <= 0.f) { return ERarity::Common; }

	const float R = FMath::FRandRange(0.f, TotalWeight);
	float Acc = 0.f;
	for (const FWeightedRarity& WR : Weights)
	{
		Acc += WR.Weight;
		if (R <= Acc)
		{
			return WR.Rarity;
		}
	}
	return Weights.Last().Rarity;
}

TArray<FModifier> UUpgradeManagerComponent::GetValidModifiers(ERarity Rarity)
{
	TArray<FModifier> Out;
	for (UUpgradeCardData* Card : AllUpgradeCards)
	{
		if (!Card || Card->Rarity != Rarity) continue;

		for (const FAttributeModifierEntry& E : Card->Modifiers)
		{
			FModifier M;
			M.ModifierID.Invalidate();
			M.Value = E.Value;
			M.Rarity = Card->Rarity;
			M.Type = E.Type;
			M.Rules = Card->Rules;
			Out.Add(M);
		}
	}
	return Out;
}

FModifier UUpgradeManagerComponent::PickRandomModifier(const TArray<FModifier>& Candidates)
{
	if (Candidates.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickRandomModifier called with empty Candidates"));
		return FModifier{};
	}
	return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

URarityData* UUpgradeManagerComponent::GetRarityData(ERarity Rarity) const
{
	for (TObjectPtr RarityData : RarityDataList)
	{
		if (RarityData && RarityData->Rarity == Rarity)
		{
			return RarityData;
		}
	}
	return nullptr;
}

void UUpgradeManagerComponent::ApplyCardToAttributes(UAttributeComponent* AttributeComp, UUpgradeCardData* Card) const
{
    if (!AttributeComp || !Card) return;

    // If this card grants an Active ability, proactively remove the currently-active ability card first,
    // so its MaxInstances refcount drops and it can return to the pool later.
    if (Card->AbilityClass && Card->AbilityType == ECardAbilityType::Active)
    {
        if (AActor* OwnerActor = AttributeComp->GetOwner())
        {
            if (UAbilityComponent* AbilityComp = OwnerActor->FindComponentByClass<UAbilityComponent>())
            {
                if (FGrantedAbilityList* ListWrapper = GrantedAbilitiesByOwner.Find(AbilityComp))
                {
                    // Find the currently active granted ability instance
                    for (int32 i = ListWrapper->Abilities.Num() - 1; i >= 0; --i)
                    {
                        const FGrantedAbilityInstance& G = ListWrapper->Abilities[i];
                        if (!G.Instance) { continue; }
                        if (G.Type == ECardAbilityType::Active && G.Instance->bIsActive)
                        {
                            UUpgradeCardData* PrevCard = G.SourceCard.Get();
                            if (PrevCard && PrevCard != Card)
                            {
                                // Remove all modifiers from previous card and revoke its ability so it re-enters the pool
                                AttributeComp->RemoveAllModifiersFromSourceObject(PrevCard);
                                RevokeAbilitiesFromCard(AbilityComp, PrevCard);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    // Enforce MaxInstances
    const int32 TotalMods = GetTotalModifierCount(Card);
    if (Card->Rules.MaxInstances > 0 && TotalMods > 0)
    {
        const int32 RefCount = AttributeComp->GetAppliedModifierRefCountForSource(Card);
        const int32 Instances = RefCount / TotalMods;
        if (Instances >= Card->Rules.MaxInstances)
        {
            UE_LOG(LogTemp, Verbose, TEXT("Card %s not applied: reached MaxInstances (%d)"), *Card->GetName(), Card->Rules.MaxInstances);
            return;
        }
    }

    const float Scale = GetRarityMultiplier(Card->Rarity);

    // Group entries by attribute
    TMap<FGameplayTag, TArray<FAttributeModifierEntry>> ModsByTag;
    for (const FAttributeModifierEntry& E : Card->Modifiers)
    {
        ModsByTag.FindOrAdd(E.TargetAttribute).Add(E);
    }

    for (const auto& KVP : ModsByTag)
    {
        EnforceCapsAndApplyForTag(AttributeComp, Card, KVP.Key, KVP.Value, Scale);
    }
}

void UUpgradeManagerComponent::RemoveCardFromAttributes(UAttributeComponent* AttributeComp, UUpgradeCardData* Card) const
{
    if (!AttributeComp || !Card) return;
    AttributeComp->RemoveAllModifiersFromSourceObject(Card);
    if (AActor* OwnerActor = AttributeComp->GetOwner())
    {
        if (UAbilityComponent* AbilityComp = OwnerActor->FindComponentByClass<UAbilityComponent>())
        {
            RevokeAbilitiesFromCard(AbilityComp, Card);
        }
        else
        {
            // Fallback: try with AttributeComp context (legacy)
            RevokeAbilitiesFromCard(AttributeComp, Card);
        }
    }
}

float UUpgradeManagerComponent::GetRarityMultiplier(ERarity InRarity) const
{
	for (const URarityData* R : RarityDataList)
	{
		if (R && R->Rarity == InRarity)
		{
			return R->ValueMultiplier;
		}
	}
	return 1.0f;
}

bool UUpgradeManagerComponent::IsCardValid(const UUpgradeCardData* Card, const UAttributeComponent* Target, int32 Floor) const
{
    if (!Card || !Target) return false;

    // Floor gating
    if (Card->Rules.MinFloor != 0 && Floor < Card->Rules.MinFloor) return false;
    if (Card->Rules.MaxFloor != 0 && Floor > Card->Rules.MaxFloor) return false;

    // MaxInstances using modifiers and ability grants
    if (Card->Rules.MaxInstances > 0)
    {
        // Compute instances from modifiers
        int32 InstancesFromMods = 0;
        const int32 TotalMods = GetTotalModifierCount(Card);
        if (TotalMods > 0)
        {
            const int32 RefCount = Target->GetAppliedModifierRefCountForSource(const_cast<UUpgradeCardData*>(Card));
            InstancesFromMods = RefCount / FMath::Max(1, TotalMods);
        }

        // Compute instances from abilities (if applicable)
        int32 InstancesFromAbilities = 0;
        if (Card->AbilityClass)
        {
            const AActor* OwnerActor = Target->GetOwner();
            if (OwnerActor)
            {
                if (const UAbilityComponent* AbilityComp = OwnerActor->FindComponentByClass<UAbilityComponent>())
                {
                    if (const FGrantedAbilityList* ListWrapper = GrantedAbilitiesByOwner.Find(const_cast<UAbilityComponent*>(AbilityComp)))
                    {
                        for (const FGrantedAbilityInstance& G : ListWrapper->Abilities)
                        {
                            if (G.SourceCard.Get() == Card)
                            {
                                InstancesFromAbilities++;
                            }
                        }
                    }
                }
            }
        }
        const int32 EffectiveInstances = FMath::Max(InstancesFromMods, InstancesFromAbilities);
        if (EffectiveInstances >= Card->Rules.MaxInstances)
        {
            return false;
        }
    }

    // Attribute caps approx
    if (!WouldRespectCapsApprox(Card, Target)) return false;

    return true;
}

TArray<UUpgradeCardData*> UUpgradeManagerComponent::RollUpgrades(UAttributeComponent* TargetAttributes, int32 NumCards)
{
    TArray<UUpgradeCardData*> Result;
    
    // Bonus contexts: consider both the owner and the attribute component to avoid mismatch
    UObject* OwnerCtx = TargetAttributes ? TargetAttributes->GetOwner() : nullptr;
    UObject* AttrCtx = TargetAttributes ? static_cast<UObject*>(TargetAttributes) : nullptr;
    TArray<UObject*> BonusContexts;
    if (OwnerCtx) BonusContexts.Add(OwnerCtx);
    if (AttrCtx) BonusContexts.Add(AttrCtx);

    auto HasAnyPendingBonus = [this](const TArray<UObject*>& Ctxs)
    {
        for (UObject* Ctx : Ctxs)
        {
            if (HasPendingHighRarityBonusFor(Ctx)) return true;
        }
        return false;
    };
    auto ConsumeFirstAvailableBonus = [this](const TArray<UObject*>& Ctxs)
    {
        for (UObject* Ctx : Ctxs)
        {
            if (HasPendingHighRarityBonusFor(Ctx))
            {
                ConsumePendingBonusFor(Ctx);
                return true;
            }
        }
        return false;
    };

    const bool bHasBonusForContext = HasAnyPendingBonus(BonusContexts);

    if (!TargetAttributes || NumCards <= 0)
    {
        if (bHasBonusForContext)
        {
            ConsumeFirstAvailableBonus(BonusContexts);
        }
        return Result;
    }

    const int32 Floor = GetCurrentFloor();

    // Track how many times a card has been picked in this roll
    TMap<UUpgradeCardData*, int32> PendingPickCounts;

    // 1) Collect guaranteed cards for this floor (ignore floor gating but respect other rules)
    bool bAnyGuaranteeForFloor = false;
    TArray<UUpgradeCardData*> Guaranteed;
    for (UUpgradeCardData* Card : AllUpgradeCards)
    {
        if (!Card) continue;
        if (IsCardGuaranteedOnFloor(Card, Floor))
        {
            bAnyGuaranteeForFloor = true; // remember there is a guarantee rule active on this floor
        }
        if (IsCardGuaranteedOnFloor(Card, Floor) && IsCardValidIgnoringFloor(Card, TargetAttributes))
        {
            Guaranteed.Add(Card);
        }
    }
    // Randomize guaranteed to avoid deterministic order
    for (int32 i = Guaranteed.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        if (i != j) { Guaranteed.Swap(i, j); }
    }
    int32 GuaranteedChosen = 0;
    for (int32 i = 0; i < NumCards && Guaranteed.Num() > 0; ++i)
    {
        Result.Add(Guaranteed[0]);
        PendingPickCounts.FindOrAdd(Guaranteed[0])++;
        Guaranteed.RemoveAtSwap(0);
        ++GuaranteedChosen;
    }
    if (bLogUpgradeRolls)
    {
        UE_LOG(LogTemp, Log, TEXT("RollUpgrades: Floor=%d, GuaranteedChosen=%d, AnyGuarantee=%s"), Floor, GuaranteedChosen, bAnyGuaranteeForFloor ? TEXT("true") : TEXT("false"));
    }
    if (Result.Num() >= NumCards)
    {
        // Append bonus slot (rarity-prioritized fallback) if requested
        if (bHasBonusForContext)
        {
            if (UUpgradeCardData* BonusCard = PickBonusCardWithFallback(TargetAttributes, Floor, PendingPickCounts))
            {
                Result.Add(BonusCard);
                PendingPickCounts.FindOrAdd(BonusCard)++;
                ConsumeFirstAvailableBonus(BonusContexts); // Only consume if we successfully added a bonus card
            }
            else 
            {
                if (bLogUpgradeRolls)
                {
                    UE_LOG(LogTemp, Log, TEXT("RollUpgrades: Bonus pool empty after fallback tiers"));
                }
                // Don't consume the bonus - let it persist for next roll attempt
            }
        }
        return Result;
    }

    // Helper to try to pick one card from a restricted rarity set, honoring validity and unique-first policy
    auto TryPickFromRarities = [&](const TArray<ERarity>& AllowedRarities) -> bool
    {
        struct FCardWeight { UUpgradeCardData* Card; float Weight; };
        TArray<FCardWeight> Pool;
        Pool.Reserve(AllUpgradeCards.Num());
        for (UUpgradeCardData* Card : AllUpgradeCards)
        {
            if (!Card) continue;
            if (!AllowedRarities.Contains(Card->Rarity)) continue;
            if (!IsCardValid(Card, TargetAttributes, Floor)) continue;
            const int32 Pending = PendingPickCounts.FindRef(Card);
            if (!CanSelectCardNow(Card, TargetAttributes, Floor, Pending)) continue;
            const float W = GetCardWeight(Card, Floor);
            if (W > 0.f) { Pool.Add({ Card, W }); }
        }
        if (Pool.Num() <= 0) return false;

        // Partition unique-first
        TArray<FCardWeight> UniquePool; UniquePool.Reserve(Pool.Num());
        TArray<FCardWeight> DuplicatePool; DuplicatePool.Reserve(Pool.Num());
        for (const FCardWeight& CW : Pool)
        {
            if (PendingPickCounts.FindRef(CW.Card) == 0) { UniquePool.Add(CW); }
            else { DuplicatePool.Add(CW); }
        }
        const TArray<FCardWeight>& ChosenPool = (UniquePool.Num() > 0) ? UniquePool : DuplicatePool;
        if (ChosenPool.Num() <= 0) return false;

        float Total = 0.f; for (const FCardWeight& CW : ChosenPool) { Total += CW.Weight; }
        if (Total <= 0.f) return false;
        const float R = FMath::FRandRange(0.f, Total);
        float Acc = 0.f; int32 PickedIndex = INDEX_NONE;
        for (int32 idx = 0; idx < ChosenPool.Num(); ++idx)
        {
            Acc += ChosenPool[idx].Weight;
            if (R <= Acc) { PickedIndex = idx; break; }
        }
        if (PickedIndex == INDEX_NONE) { PickedIndex = ChosenPool.Num() - 1; }
        Result.Add(ChosenPool[PickedIndex].Card);
        PendingPickCounts.FindOrAdd(ChosenPool[PickedIndex].Card)++;
        return true;
    };

    // If there is any guarantee active on this floor, prefer to backfill remaining slots with Common then Uncommon
    if (bAnyGuaranteeForFloor && Result.Num() < NumCards)
    {
        // Fill remaining using Common first, then Uncommon
        while (Result.Num() < NumCards)
        {
            bool bPicked = false;
            if (!bPicked) { bPicked = TryPickFromRarities({ ERarity::Common }); }
            if (!bPicked) { bPicked = TryPickFromRarities({ ERarity::Uncommon }); }
            if (!bPicked) { break; } // stop if nothing of preferred rarities is available
        }

        // If still not enough, fall back to general selection (any rarity) below
    }

    // 2) Sample WITHOUT replacement preferentially (avoid duplicates). Only allow duplicates if unique pool is exhausted.
    struct FCardWeight { UUpgradeCardData* Card; float Weight; };
    int32 SafetyCounter = 0;
    while (Result.Num() < NumCards)
    {
        TArray<FCardWeight> Pool;
        for (UUpgradeCardData* Card : AllUpgradeCards)
        {
            if (!Card) continue;
            // Must be valid for this floor and target
            if (!IsCardValid(Card, TargetAttributes, Floor)) continue;
            // Respect MaxInstances considering how many times we've already picked it in this roll
            const int32 Pending = PendingPickCounts.FindRef(Card);
            if (!CanSelectCardNow(Card, TargetAttributes, Floor, Pending)) continue;

            const float W = GetCardWeight(Card, Floor);
            if (W > 0.f)
            {
                Pool.Add({ Card, W });
            }
        }

        // Partition into unique-first and duplicates-allowed pools
        TArray<FCardWeight> UniquePool; UniquePool.Reserve(Pool.Num());
        TArray<FCardWeight> DuplicatePool; DuplicatePool.Reserve(Pool.Num());
        for (const FCardWeight& CW : Pool)
        {
            if (PendingPickCounts.FindRef(CW.Card) == 0)
            {
                UniquePool.Add(CW);
            }
            else
            {
                DuplicatePool.Add(CW);
            }
        }

        const bool bUseUnique = UniquePool.Num() > 0;
        const TArray<FCardWeight>& ChosenPool = bUseUnique ? UniquePool : DuplicatePool;

        if (bLogUpgradeRolls)
        {
            UE_LOG(LogTemp, Log, TEXT("RollUpgrades: WeightedPool=%d (unique=%d, duplicate=%d) %s"), Pool.Num(), UniquePool.Num(), DuplicatePool.Num(), bUseUnique ? TEXT("[UNIQUE-FIRST]") : TEXT("[DUPLICATE-FALLBACK]"));
        }

        if (ChosenPool.Num() <= 0)
        {
            break; // Can't fill further while respecting rules
        }

        float Total = 0.f; for (const FCardWeight& CW : ChosenPool) { Total += CW.Weight; }
        if (Total <= 0.f) { break; }

        const float R = FMath::FRandRange(0.f, Total);
        float Acc = 0.f;
        int32 PickedIndex = INDEX_NONE;
        for (int32 idx = 0; idx < ChosenPool.Num(); ++idx)
        {
            Acc += ChosenPool[idx].Weight;
            if (R <= Acc)
            {
                PickedIndex = idx;
                break;
            }
        }
        if (PickedIndex == INDEX_NONE)
        {
            PickedIndex = ChosenPool.Num() - 1;
        }

        if (bLogUpgradeRolls)
        {
            UE_LOG(LogTemp, Log, TEXT("RollUpgrades: Picked %s (W=%.2f)%s"), *ChosenPool[PickedIndex].Card->GetName(), ChosenPool[PickedIndex].Weight, bUseUnique ? TEXT(" [UNIQUE]") : TEXT(""));
        }
        Result.Add(ChosenPool[PickedIndex].Card);
        PendingPickCounts.FindOrAdd(ChosenPool[PickedIndex].Card)++;

        // Safety to avoid potential infinite loops if something goes wrong
        if (++SafetyCounter > 1024) { break; }
    }

    // 3) After normal pulls, append bonus slot (rarity-prioritized fallback) if requested
    if (bHasBonusForContext)
    {
        if (UUpgradeCardData* BonusCard = PickBonusCardWithFallback(TargetAttributes, Floor, PendingPickCounts))
        {
            Result.Add(BonusCard);
            PendingPickCounts.FindOrAdd(BonusCard)++;
            ConsumeFirstAvailableBonus(BonusContexts); // Only consume if we successfully added a bonus card
        }
        else 
        {
            if (bLogUpgradeRolls)
            {
                UE_LOG(LogTemp, Log, TEXT("RollUpgrades: Bonus pool empty after fallback tiers"));
            }
            // Don't consume the bonus - let it persist for next roll attempt
        }
    }

    return Result;
}

TArray<UUpgradeCardData*> UUpgradeManagerComponent::GetGuaranteedCardsForFloor(UAttributeComponent* TargetAttributes, int32 Floor) const
{
	TArray<UUpgradeCardData*> Out;
	if (!TargetAttributes || Floor <= 0) return Out;
	for (UUpgradeCardData* Card : AllUpgradeCards)
	{
		if (!Card) continue;
		if (IsCardGuaranteedOnFloor(Card, Floor) && IsCardValidIgnoringFloor(Card, TargetAttributes))
		{
			Out.Add(Card);
		}
	}
	return Out;
}

EBenefitDirection UUpgradeManagerComponent::ResolveDirectionForTag(FGameplayTag Tag,
	TArray<FAttributeModifierEntry> AttributeModifierEntries) const
{
	for (const FAttributeModifierEntry& E : AttributeModifierEntries)
	{
		if (E.TargetAttribute == Tag)
		{
			return E.BenefitDirection;
		}
	}
	return EBenefitDirection::InferFromAttribute;
}

float UUpgradeManagerComponent::GetCardWeight(const UUpgradeCardData* Card, int32 Floor) const
{
	if (!Card) return 0.f;
	URarityData* RData = GetRarityData(Card->Rarity);
	float Weight = (RData ? RData->RollWeight : 1.f);

	// Boost if floor is within card range
	const FModifierRule& Rule = Card->Rules;
	const bool bHasRange = (Rule.MinFloor != 0 || Rule.MaxFloor != 0);
	if (bHasRange && Floor >= 0)
	{
		const bool bInRange = (Rule.MinFloor == 0 || Floor >= Rule.MinFloor) && (Rule.MaxFloor == 0 || Floor <= Rule.MaxFloor);
		if (bInRange)
		{
			Weight *= 1.25f;
		}
	}
	return Weight;
}

bool UUpgradeManagerComponent::WouldRespectCapsApprox(const UUpgradeCardData* Card, const UAttributeComponent* Target) const
{
	if (!Card || !Target) return true;

	const float Scale = GetRarityMultiplier(Card->Rarity);

	// Gather unified rows from entries
	TMap<FGameplayTag, float> AddByTag;
	TMap<FGameplayTag, float> MulByTag;
	TMap<FGameplayTag, TOptional<float>> OverrideByTag;

	for (const FAttributeModifierEntry& E : Card->Modifiers)
	{
		switch (E.Type)
		{
		case EModificationType::Addition:
			AddByTag.FindOrAdd(E.TargetAttribute) += (E.Value * Scale);
			break;
		case EModificationType::Subtraction:
			AddByTag.FindOrAdd(E.TargetAttribute) -= (E.Value * Scale);
			break;
		case EModificationType::Multiplication:
			{
				float& Factor = MulByTag.FindOrAdd(E.TargetAttribute);
				if (Factor == 0.f) Factor = 1.f;
				Factor *= (E.Value);
			}
			break;
		case EModificationType::Override:
			OverrideByTag.FindOrAdd(E.TargetAttribute) = (E.Value * Scale);
			break;
		}
	}

	// Per-card MIN caps
	for (const FAttributeCap& Cap : Card->Rules.AttributeCaps)
	{
		if (!Cap.Attribute.IsValid()) continue;
		const bool bAffected = AddByTag.Contains(Cap.Attribute) || MulByTag.Contains(Cap.Attribute) || (OverrideByTag.Contains(Cap.Attribute) && OverrideByTag[Cap.Attribute].IsSet());
		if (!bAffected) continue;

		float Current = Target->GetAttributeValue(Cap.Attribute);
		float Add = AddByTag.FindRef(Cap.Attribute);
		float Mul = MulByTag.Contains(Cap.Attribute) ? MulByTag.FindRef(Cap.Attribute) : 1.f;
		TOptional<float> Override = OverrideByTag.Contains(Cap.Attribute) ? OverrideByTag[Cap.Attribute] : TOptional<float>();

		float NewVal = Override.IsSet() ? Override.GetValue() : (Current + Add) * Mul;
		if (Cap.bEnforceMin && NewVal < Cap.MinValue) { return false; }
	}

	// Attribute-level max clamps
	TSet<FGameplayTag> AffectedTags;
	AddByTag.GetKeys(AffectedTags);
	for (const auto& KVP : MulByTag) { AffectedTags.Add(KVP.Key); }
	for (const auto& KVP : OverrideByTag) { if (KVP.Value.IsSet()) { AffectedTags.Add(KVP.Key); } }

	for (const FGameplayTag& Tag : AffectedTags)
	{
		const float Current = Target->GetAttributeValue(Tag);
		const float Add = AddByTag.FindRef(Tag);
		const float Mul = MulByTag.Contains(Tag) ? MulByTag.FindRef(Tag) : 1.f;
		const TOptional<float> Override = OverrideByTag.Contains(Tag) ? OverrideByTag[Tag] : TOptional<float>();
		const float NewVal = Override.IsSet() ? Override.GetValue() : (Current + Add) * Mul;

		if (Target->IsAttributeUsingMaxClamp(Tag))
		{
			const float MaxClamp = Target->GetAttributeMaxClampValue(Tag);
			if (NewVal > MaxClamp)
			{
				return false;
			}
		}
	}

	return true;
}

bool UUpgradeManagerComponent::EnforceCapsAndApplyForTag(UAttributeComponent* AttributeComp,
	UUpgradeCardData* Card,
	FGameplayTag Tag,
	const TArray<FAttributeModifierEntry>& ModsForTag,
	float Scale) const
{
	if (!AttributeComp || !Card || !Tag.IsValid() || ModsForTag.Num() == 0) return false;

	// Compute projected new value
	float Current = AttributeComp->GetAttributeValue(Tag);
	float Add = 0.f;
	float Mul = 1.f;
	TOptional<float> Override;
	for (const FAttributeModifierEntry& Row : ModsForTag)
	{
		switch (Row.Type)
		{
		case EModificationType::Addition: Add += (Row.Value * Scale); break;
		case EModificationType::Subtraction: Add -= (Row.Value * Scale); break;
		case EModificationType::Multiplication: Mul *= Row.Value; break;
		case EModificationType::Override: Override = (Row.Value * Scale); break;
		}
	}
	float NewVal = Override.IsSet() ? Override.GetValue() : (Current + Add) * Mul;

	// Check per-card MIN caps only
	for (const FAttributeCap& Cap : Card->Rules.AttributeCaps)
	{
		if (Cap.Attribute != Tag) continue;
		if (Cap.bEnforceMin && NewVal < Cap.MinValue)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Card %s skipped for tag %s due to Min cap (%f < %f)"), *Card->GetName(), *Tag.ToString(), NewVal, Cap.MinValue);
			return false;
		}
	}

	// Attribute-level max clamp enforcement
	if (AttributeComp->IsAttributeUsingMaxClamp(Tag))
	{
		const float MaxClamp = AttributeComp->GetAttributeMaxClampValue(Tag);
		if (NewVal > MaxClamp)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Card %s skipped for tag %s due to Attribute Max clamp (%f > %f)"), *Card->GetName(), *Tag.ToString(), NewVal, MaxClamp);
			return false;
		}
	}

	// Apply modifiers
	for (const FAttributeModifierEntry& Row : ModsForTag)
	{
		FModifier M;
		M.ModifierID = FGuid::NewGuid();
		M.Value = (Row.Type == EModificationType::Multiplication) ? Row.Value : (Row.Value * Scale);
		M.Rarity = Card->Rarity;
		M.Type = Row.Type;
		M.Rules = Card->Rules;

		AttributeComp->AddModifier(Tag, M);
		AttributeComp->RegisterAppliedModifier(Card, Tag, M.ModifierID);
	}
	return true;
}

// ADD: required by header (guarantee check)
bool UUpgradeManagerComponent::IsCardGuaranteedOnFloor(const UUpgradeCardData* Card, int32 Floor) const
{
    if (!Card || Floor <= 0) return false;
    // Specific floors take precedence
    for (int32 F : Card->Rules.GuaranteedFloors)
    {
        if (F == Floor) return true;
    }
    // Simple periodic guarantee (0 disables)
    if (Card->Rules.GuaranteeEveryXFloors > 0)
    {
        return (Floor % Card->Rules.GuaranteeEveryXFloors) == 0;
    }
    return false;
}

// ADD: required by header (validity w/o floor gating)
bool UUpgradeManagerComponent::IsCardValidIgnoringFloor(const UUpgradeCardData* Card, const UAttributeComponent* Target) const
{
	if (!Card || !Target) return false;

	// MaxInstances using modifiers and ability grants
	if (Card->Rules.MaxInstances > 0)
	{
		int32 InstancesFromMods = 0;
		const int32 TotalMods = GetTotalModifierCount(Card);
		if (TotalMods > 0)
		{
			const int32 RefCount = Target->GetAppliedModifierRefCountForSource(const_cast<UUpgradeCardData*>(Card));
			InstancesFromMods = RefCount / FMath::Max(1, TotalMods);
		}
		int32 InstancesFromAbilities = 0;
		if (Card->AbilityClass)
		{
			const AActor* OwnerActor = Target->GetOwner();
			if (OwnerActor)
			{
				if (const UAbilityComponent* AbilityComp = OwnerActor->FindComponentByClass<UAbilityComponent>())
				{
					if (const FGrantedAbilityList* ListWrapper = GrantedAbilitiesByOwner.Find(const_cast<UAbilityComponent*>(AbilityComp)))
					{
						for (const FGrantedAbilityInstance& G : ListWrapper->Abilities)
						{
							if (G.SourceCard.Get() == Card)
							{
								InstancesFromAbilities++;
							}
						}
					}
				}
			}
		}
		const int32 EffectiveInstances = FMath::Max(InstancesFromMods, InstancesFromAbilities);
		if (EffectiveInstances >= Card->Rules.MaxInstances)
		{
			return false;
		}
	}

	// Attribute caps approx
	if (!WouldRespectCapsApprox(Card, Target)) return false;

	return true;
}

// ADD: required by header (total modifier count)
int32 UUpgradeManagerComponent::GetTotalModifierCount(const UUpgradeCardData* Card) const
{
	return Card ? Card->Modifiers.Num() : 0;
}

// ADD: required by header (card preview)
TArray<FAttributePreviewResult> UUpgradeManagerComponent::PreviewCard(UAttributeComponent* TargetAttributes, UUpgradeCardData* Card) const
{
	TArray<FAttributePreviewResult> Out;
	if (!TargetAttributes || !Card) return Out;

	const float Scale = GetRarityMultiplier(Card->Rarity);

	// Group entries by attribute
	TMap<FGameplayTag, TArray<FAttributeModifierEntry>> ModsByTag;
	for (const FAttributeModifierEntry& E : Card->Modifiers)
	{
		ModsByTag.FindOrAdd(E.TargetAttribute).Add(E);
	}
	

	for (const auto& KVP : ModsByTag)
	{
		const FGameplayTag Tag = KVP.Key;
		const TArray<FAttributeModifierEntry>& Entries = KVP.Value;

		// Convert entries to runtime modifiers expected by AttributeComponent
		TArray<FModifier> RuntimeMods;
		RuntimeMods.Reserve(Entries.Num());
		for (const FAttributeModifierEntry& E : Entries)
		{
			FModifier M;
			M.ModifierID.Invalidate();
			M.Value = (E.Type == EModificationType::Multiplication) ? E.Value : (E.Value * Scale);
			M.Rarity = Card->Rarity;
			M.Type = E.Type;
			M.Rules = Card->Rules;
			RuntimeMods.Add(M);
		}

		const FAttributePreviewResult Base = TargetAttributes->PreviewApplyModifiersForTag(Tag, RuntimeMods);

		// Resolve display direction directly from Entries (no DataTable rows)
		FAttributePreviewResult Final = Base;
		Final.ResolvedDirection = ResolveDirectionForTag(Tag, Entries);
		Final.ModifierTextOverride = Card->ModifierTextOverride;
		// NEW: propagate presence of Override-type modifiers from the card entries for this tag
		for (const FAttributeModifierEntry& E : Entries)
		{
			if (E.Type == EModificationType::Override)
			{
				Final.bHasOverrideChange = true;
				break;
			}
		}
		Out.Add(MoveTemp(Final));
	}

	return Out;
}

FText UUpgradeManagerComponent::GetCardAbilityDisplayName(UUpgradeCardData* Card)
{
    if (!Card || !Card->AbilityClass) return FText();
    if (!Card->AbilityDisplayNameOverride.IsEmpty())
    {
        return Card->AbilityDisplayNameOverride;
    }
    // Use the class default object to retrieve a user-facing name (works in runtime without editor-only APIs)
    if (const UGameplayAbilityObject* CDO = Card->AbilityClass->GetDefaultObject<UGameplayAbilityObject>())
    {
        const FText Name = CDO->GetAbilityDisplayName();
        if (!Name.IsEmpty())
        {
            return Name;
        }
    }
    // Fallback: derive from class name (strip common prefixes like BP_)
    FString ClassName = Card->AbilityClass->GetName();
    ClassName.ReplaceInline(TEXT("BP_"), TEXT(""));
    return FText::FromString(ClassName);
}

FText UUpgradeManagerComponent::GetCardAbilityDisplayLine(UUpgradeCardData* Card)
{
    if (!Card || !Card->AbilityClass) return FText();

    const FText Name = GetCardAbilityDisplayName(Card);

    FText Desc;
    if (!Card->AbilityDescriptionOverride.IsEmpty())
    {
        Desc = Card->AbilityDescriptionOverride;
    }
    else if (const UGameplayAbilityObject* CDO = Card->AbilityClass->GetDefaultObject<UGameplayAbilityObject>())
    {
        Desc = CDO->GetAbilityDisplayDescription();
    }

    if (!Desc.IsEmpty())
    {
        return FText::FromString(Name.ToString() + TEXT(": ") + Desc.ToString());
    }
    return Name;
}

UGameplayAbilityObject* UUpgradeManagerComponent::GetActiveAbilityForOwner(UObject* OwnerContext) const
{
	if (!OwnerContext) return nullptr;
	if (FGrantedAbilityList* ListWrapper = GrantedAbilitiesByOwner.Find(OwnerContext))
	{
		TArray<FGrantedAbilityInstance>& Arr = ListWrapper->Abilities;
		for (int32 i = Arr.Num() - 1; i >= 0; --i)
		{
			FGrantedAbilityInstance& G = Arr[i];
			if (!G.Instance)
			{
				Arr.RemoveAt(i);
				continue;
			}
			if (G.Type == ECardAbilityType::Active && G.Instance->bIsActive)
			{
				return G.Instance;
			}
		}
	}
	return nullptr;
}

TArray<UGameplayAbilityObject*> UUpgradeManagerComponent::GetPassiveAbilitiesForOwner(UObject* OwnerContext) const
{
	TArray<UGameplayAbilityObject*> Out;
	if (!OwnerContext) return Out;
	if (FGrantedAbilityList* ListWrapper = GrantedAbilitiesByOwner.Find(OwnerContext))
	{
		TArray<FGrantedAbilityInstance>& Arr = ListWrapper->Abilities;
		for (int32 i = Arr.Num() - 1; i >= 0; --i)
		{
			FGrantedAbilityInstance& G = Arr[i];
			if (!G.Instance)
			{
				Arr.RemoveAt(i);
				continue;
			}
			if (G.Type == ECardAbilityType::Passive && G.Instance->bIsActive)
			{
				Out.Add(G.Instance);
			}
		}
	}
	return Out;
}

void UUpgradeManagerComponent::GrantAbilityForCard(UAbilityComponent* AbilityComp, UUpgradeCardData* Card) const
{
    if (!AbilityComp || !Card || !Card->AbilityClass) return;

    // If we are granting a new Active ability, clear the current one and its card modifiers first so it can return to the pool
    if (Card->AbilityType == ECardAbilityType::Active)
    {
        if (FGrantedAbilityList* ListWrapper = GrantedAbilitiesByOwner.Find(AbilityComp))
        {
            for (int32 i = ListWrapper->Abilities.Num() - 1; i >= 0; --i)
            {
                const FGrantedAbilityInstance& G = ListWrapper->Abilities[i];
                if (G.Instance && G.Type == ECardAbilityType::Active && G.Instance->bIsActive)
                {
                    UUpgradeCardData* PrevCard = G.SourceCard.Get();
                    if (PrevCard && PrevCard != Card)
                    {
                        // Remove modifiers and revoke the previous active ability
                        if (AActor* OwnerActor = AbilityComp->GetOwner())
                        {
                            if (UAttributeComponent* AttrComp = OwnerActor->FindComponentByClass<UAttributeComponent>())
                            {
                                AttrComp->RemoveAllModifiersFromSourceObject(PrevCard);
                            }
                        }
                        RevokeAbilitiesFromCard(AbilityComp, PrevCard);
                    }
                    break;
                }
            }
        }
    }

    // Create the ability instance. Prefer the AbilityComponent as the outer to ease GC and ownership.
    UObject* OuterForAbility = AbilityComp ? static_cast<UObject*>(AbilityComp) : AbilityComp;
    UGameplayAbilityObject* Ability = NewObject<UGameplayAbilityObject>(OuterForAbility, Card->AbilityClass);
    if (!Ability)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to instantiate ability %s for card %s"), *Card->AbilityClass->GetName(), *Card->GetName());
        return;
    }
    Ability->Activate(AbilityComp);

    if (AbilityComp)
    {
        if (Card->AbilityType == ECardAbilityType::Passive)
        {
            AbilityComp->AddPassiveAbility(Ability);
        }
        else // Active ability: choose a slot; prefer empty slots
        {
            AbilityComp->SetCurrentCombatAbility(Ability);
        }
    }

    // Track for later revocation/queries
    FGrantedAbilityList& ListWrapper = GrantedAbilitiesByOwner.FindOrAdd(AbilityComp);
    FGrantedAbilityInstance Entry; Entry.Instance = Ability; Entry.SourceCard = Card; Entry.Type = Card->AbilityType;
    ListWrapper.Abilities.Add(Entry);
}

void UUpgradeManagerComponent::RevokeAbilitiesFromCard(UObject* OwnerContext, UUpgradeCardData* Card) const
{
	if (!OwnerContext || !Card) return;

	UAbilityComponent* AbilityComp = nullptr;
	if (AActor* AsActor = Cast<AActor>(OwnerContext))
	{
		AbilityComp = AsActor->FindComponentByClass<UAbilityComponent>();
	}
	else if (UActorComponent* AsComp = Cast<UActorComponent>(OwnerContext))
	{
		if (AActor* Owner = AsComp->GetOwner())
		{
			AbilityComp = Owner->FindComponentByClass<UAbilityComponent>();
		}
	}

	if (FGrantedAbilityList* ListWrapper = GrantedAbilitiesByOwner.Find(OwnerContext))
	{
		TArray<FGrantedAbilityInstance>& Arr = ListWrapper->Abilities;
		for (int32 i = Arr.Num() - 1; i >= 0; --i)
		{
			FGrantedAbilityInstance& G = Arr[i];
			if (G.SourceCard.Get() == Card)
			{
				if (AbilityComp && G.Instance)
				{
					if (G.Type == ECardAbilityType::Passive)
					{
						//AbilityComp->RemovePassiveAbility(G.Instance);
					}
					else
					{
						if (AbilityComp->GetCurrentSupportAbility() == G.Instance)
						{
							//AbilityComp->ClearCurrentSupportAbility();
						}
						else if (AbilityComp->GetCurrentCombatAbility() == G.Instance)
						{
							//AbilityComp->ClearCurrentCombatAbility();
						}
						else
						{
							G.Instance->Deactivate();
						}
					}
				}
				else if (G.Instance)
				{
					G.Instance->Deactivate();
				}
				Arr.RemoveAt(i);
			}
		}
		if (Arr.Num() == 0)
		{
			GrantedAbilitiesByOwner.Remove(OwnerContext);
		}
	}
}

void UUpgradeManagerComponent::RevokeAllAbilitiesForOwner(UObject* OwnerContext) const
{
	if (!OwnerContext) return;
	UAbilityComponent* AbilityComp = nullptr;
	if (AActor* AsActor = Cast<AActor>(OwnerContext)) { AbilityComp = AsActor->FindComponentByClass<UAbilityComponent>(); }
	else if (UActorComponent* AsComp = Cast<UActorComponent>(OwnerContext)) { if (AActor* Owner = AsComp->GetOwner()) { AbilityComp = Owner->FindComponentByClass<UAbilityComponent>(); } }

	if (FGrantedAbilityList* ListWrapper = GrantedAbilitiesByOwner.Find(OwnerContext))
	{
		TArray<FGrantedAbilityInstance>& Arr = ListWrapper->Abilities;
		for (int32 i = Arr.Num() - 1; i >= 0; --i)
		{
			FGrantedAbilityInstance& G = Arr[i];
			if (AbilityComp && G.Instance)
			{
				// if (G.Type == ECardAbilityType::Passive) { AbilityComp->RemovePassiveAbility(G.Instance); }
				// else {
				// 	if (AbilityComp->GetCurrentSupportAbility() == G.Instance) { AbilityComp->ClearCurrentSupportAbility(); }
				// 	else if (AbilityComp->GetCurrentCombatAbility() == G.Instance) { AbilityComp->ClearCurrentCombatAbility(); }
				// 	else { G.Instance->Deactivate(); }
				// }
			}
			else if (G.Instance)
			{
				G.Instance->Deactivate();
			}
		}
	}
	GrantedAbilitiesByOwner.Remove(OwnerContext);
}

void UUpgradeManagerComponent::RevokeAllAbilities() const
{
	for (auto& KVP : GrantedAbilitiesByOwner)
	{
		UObject* OwnerContext = KVP.Key.Get();
		if (!OwnerContext) { continue; }
		RevokeAllAbilitiesForOwner(OwnerContext);
	}
	GrantedAbilitiesByOwner.Empty();
}

void UUpgradeManagerComponent::ApplyAbilitiesFromCard(UAbilityComponent* AbilityComp,
                                                      UUpgradeCardData* Card) const
{
	if (!AbilityComp || !Card || !Card->AbilityClass) return;
	GrantAbilityForCard(AbilityComp, Card);
}

void UUpgradeManagerComponent::RemoveAbilitiesFromCard(UAbilityComponent* AbilityComp,
                                                       UUpgradeCardData* Card) const
{
	if (!AbilityComp || !Card) return;
	RevokeAbilitiesFromCard(AbilityComp, Card);
}

static FString SplitCamelCase(const FString& In)
{
	FString Out;
	Out.Reserve(In.Len() + In.Len() / 3);
	for (int32 i = 0; i < In.Len(); ++i)
	{
		const TCHAR C = In[i];
		const bool bIsUpper = FChar::IsUpper(C);
		const bool bIsAlphaNum = FChar::IsAlnum(C);
		if (i > 0)
		{
			const TCHAR Prev = In[i - 1];
			const bool bPrevIsLower = FChar::IsLower(Prev);
			const bool bPrevIsAlphaNum = FChar::IsAlnum(Prev);
			if (bIsUpper && bPrevIsLower && bPrevIsAlphaNum)
			{
				Out.AppendChar(' ');
			}
		}
		if (bIsAlphaNum)
		{
			Out.AppendChar(C);
		}
		else
		{
			Out.AppendChar(' ');
		}
	}
	Out.TrimStartAndEndInline();
	return Out;
}

static FText FormatSignedFloat(float Value, int32 MaxDecimals)
{
	MaxDecimals = FMath::Clamp(MaxDecimals, 0, 6);
	FNumberFormattingOptions Opts;
	Opts.MaximumFractionalDigits = MaxDecimals;
	Opts.MinimumFractionalDigits = 0;
	const bool bNeg = Value < 0.f;
	const float AbsV = FMath::Abs(Value);
	FText Number = FText::AsNumber(AbsV, &Opts);
	FString S = (bNeg ? TEXT("-") : TEXT("+"));
	S += Number.ToString();
	return FText::FromString(MoveTemp(S));
}

// Infer direction from attribute tag text when rows do not provide explicit direction
static EBenefitDirection InferDirectionFromTag(FGameplayTag AttributeTag)
{
	if (!AttributeTag.IsValid())
	{
		return EBenefitDirection::HigherIsBetter;
	}
	FString S = AttributeTag.ToString().ToLower();
	// Heuristics: treat common time-like metrics as LowerIsBetter
	if (S.Contains(TEXT("cooldown")) || S.Contains(TEXT("delay")) || S.Contains(TEXT("duration")) || S.Contains(TEXT("time")))
	{
		return EBenefitDirection::LowerIsBetter;
	}
	// Otherwise default to higher-is-better
	return EBenefitDirection::HigherIsBetter;
}

static bool IsChangeBeneficial(float Delta, EBenefitDirection Dir)
{
	if (FMath::IsNearlyZero(Delta))
	{
		return false;
	}
	switch (Dir)
	{
	case EBenefitDirection::LowerIsBetter:
		return Delta < 0.f;
	case EBenefitDirection::HigherIsBetter:
		return Delta > 0.f;
	case EBenefitDirection::InferFromAttribute:
	default:
		return Delta > 0.f; // shouldn't hit; callers should resolve first
	}
}

FText UUpgradeManagerComponent::GetFriendlyAttributeName(FGameplayTag AttributeTag, const FText& NameOverride)
{
	if (!AttributeTag.IsValid())
	{
		return FText::GetEmpty();
	}
	// check for card attribute name override first
	if (NameOverride.IsEmpty() == false)
	{
		return NameOverride;
	}
	FString TagStr = AttributeTag.ToString();
	TArray<FString> Parts;
	TagStr.ParseIntoArray(Parts, TEXT("."), true);
	if (Parts.Num() == 0)
	{
		return FText::FromString(TagStr);
	}
	// Prefer last part as the core name; drop leading "Attribute" if present
	int32 StartIdx = 0;
	if (Parts[0].Equals(TEXT("Attribute"), ESearchCase::IgnoreCase))
	{
		StartIdx = 1;
	}
	FString Core = Parts.IsValidIndex(Parts.Num() - 1) ? Parts.Last() : Parts[StartIdx];
	Core.ReplaceInline(TEXT("_"), TEXT(" "));
	const FString Spaced = SplitCamelCase(Core);
	return FText::FromString(Spaced);
}

FText UUpgradeManagerComponent::FormatAttributeChange(FGameplayTag AttributeTag,
                                                      float OldValue,
                                                      float NewValue,
                                                      bool /*bPreferPercent*/,
                                                      int32 MaxDecimals,
                                                      bool bAnnotateClamps,
                                                      bool bClampedByAttributeMax,
                                                      const FText& NameOverride)
{
	const float Delta = NewValue - OldValue;
	if (FMath::IsNearlyZero(Delta))
	{
		return FText();
	}
	// Flat-only formatting (add/sub scenario)
	FText FlatText = FormatSignedFloat(Delta, MaxDecimals);
	if (bClampedByAttributeMax)
	{
		FNumberFormattingOptions Opts; Opts.MaximumFractionalDigits = MaxDecimals; Opts.MinimumFractionalDigits = 0;
		FText OldText = FText::AsNumber(OldValue, &Opts);
		FText NewText = FText::AsNumber(NewValue, &Opts);
		FString Line = FString::Printf(TEXT("%s %s → %s"), *GetFriendlyAttributeName(AttributeTag, NameOverride).ToString(), *OldText.ToString(), *NewText.ToString());
		if (bAnnotateClamps)
		{
			Line.Append(TEXT(" (MAX)"));
		}
		return FText::FromString(MoveTemp(Line));
	}
	return FText::FromString(FString::Printf(TEXT("%s %s"), *GetFriendlyAttributeName(AttributeTag, NameOverride).ToString(), *FlatText.ToString()));
}

// Helper: decide if result is pure multiplicative (no additive) and with real factor change
static bool IsPureMultiplicative(const FAttributePreviewResult& R)
{
	return R.bHasMultiplicativeChange && !R.bHasAdditiveChange && !FMath::IsNearlyEqual(R.NetMultiplicativeFactor, 1.f);
}


void UUpgradeManagerComponent::GetCardPreviewTexts(const TArray<FAttributePreviewResult>& PreviewResults,
                                                   FText& PositiveText,
                                                   FText& NegativeText,
                                                   FText& CombinedText,
                                                   bool /*bPreferPercent*/,
                                                   int32 MaxDecimals,
                                                   bool bAnnotateClamps)
{
	PositiveText = FText(); NegativeText = FText(); CombinedText = FText();
	if (PreviewResults.Num() == 0) return;
	TArray<FString> PositiveLines; TArray<FString> NegativeLines; float MaxPosAbs=0.f, MaxNegAbs=0.f; int32 MaxPosIdx=INDEX_NONE, MaxNegIdx=INDEX_NONE;
	for (const FAttributePreviewResult& R : PreviewResults) // BEGIN_FORMAT_LOOP
	{
		const float Delta = R.NewValue - R.OldValue; // incremental change
		EBenefitDirection Dir = (R.ResolvedDirection != EBenefitDirection::InferFromAttribute) ? R.ResolvedDirection : InferDirectionFromTag(R.Attribute);
		FString FormattedLine;
		bool bBeneficial = false;

		if (R.bUnderlyingChangeMaskedByRounding)
		{
			if (IsPureMultiplicative(R) && !FMath::IsNearlyZero(R.RawOldValue))
			{
				const float RawDelta = R.RawNewValue - R.RawOldValue;
				float Pct = ((R.RawNewValue / R.RawOldValue) - 1.f) * 100.f;
				FText PctText = FormatSignedFloat(Pct, FMath::Clamp(MaxDecimals,0,2));
				FormattedLine = FString::Printf(TEXT("%s %s%%"),
					*GetFriendlyAttributeName(R.Attribute, R.ModifierTextOverride).ToString(),
					*PctText.ToString());
				bBeneficial = IsChangeBeneficial(RawDelta, Dir);
			}
			else { continue; }
		}
		else if (!FMath::IsNearlyZero(Delta))
		{
			if (IsPureMultiplicative(R) && !FMath::IsNearlyZero(R.OldValue))
			{
				float Pct = ((R.NewValue / R.OldValue) - 1.f) * 100.f;
				FText PctText = FormatSignedFloat(Pct, FMath::Clamp(MaxDecimals,0,2));
				FormattedLine = FString::Printf(TEXT("%s %s%%"),
					*GetFriendlyAttributeName(R.Attribute, R.ModifierTextOverride).ToString(),
					*PctText.ToString());
				bBeneficial = IsChangeBeneficial(Delta, Dir);
			}
			else
			{
				FText Flat = FormatSignedFloat(Delta, MaxDecimals);
				FormattedLine = FString::Printf(TEXT("%s %s"), *GetFriendlyAttributeName(R.Attribute, R.ModifierTextOverride).ToString(), *Flat.ToString());
				bBeneficial = IsChangeBeneficial(Delta, Dir);
			}
		}
		// NEW: If an Override was applied but resulted in no visible delta, still show a positive line
		else if (R.bHasOverrideChange)
		{
			FNumberFormattingOptions Opts; Opts.MaximumFractionalDigits = MaxDecimals; Opts.MinimumFractionalDigits = 0;
			FText NewValText = FText::AsNumber(R.NewValue, &Opts);
			FormattedLine = FString::Printf(TEXT("%s SET TO %s"), *GetFriendlyAttributeName(R.Attribute, R.ModifierTextOverride).ToString(), *NewValText.ToString());
			bBeneficial = true; // classify as positive per requirement
		}
		else if (R.ClampApplied != EAttributeClampMode::None)
		{
			// Clamp with no visible delta -> show attribute + clamp annotation only
			FormattedLine = GetFriendlyAttributeName(R.Attribute, R.ModifierTextOverride).ToString();
			if (bAnnotateClamps)
			{
				if (R.ClampApplied == EAttributeClampMode::Max) FormattedLine.Append(TEXT(" (MAX)"));
				else if (R.ClampApplied == EAttributeClampMode::Min) FormattedLine.Append(TEXT(" (MIN)"));
			}
			bBeneficial = (R.ClampApplied == EAttributeClampMode::Max) ? (Dir == EBenefitDirection::HigherIsBetter) : (Dir == EBenefitDirection::LowerIsBetter);
		}
		if (FormattedLine.IsEmpty()) continue;
		float Significance = FMath::Abs(R.RawNewValue - R.RawOldValue);
		if (bBeneficial)
		{
			PositiveLines.Add(FormattedLine);
			if (Significance > MaxPosAbs)
			{
				MaxPosAbs=Significance;
				MaxPosIdx=PositiveLines.Num()-1;
			}
		}
		else
		{
			NegativeLines.Add(FormattedLine); if (Significance > MaxNegAbs){MaxNegAbs=Significance;MaxNegIdx=NegativeLines.Num()-1;}
		}
	}
	if (MaxPosIdx!=INDEX_NONE) PositiveText=FText::FromString(PositiveLines[MaxPosIdx]);
	else if (PositiveLines.Num() > 0) PositiveText = FText::FromString(PositiveLines[0]); // fallback when significance ties at 0
	if (MaxNegIdx!=INDEX_NONE) NegativeText=FText::FromString(NegativeLines[MaxNegIdx]);
	else if (NegativeLines.Num() > 0) NegativeText = FText::FromString(NegativeLines[0]);
	TArray<FString> All; All.Append(PositiveLines); All.Append(NegativeLines); if (All.Num()>0) CombinedText=FText::FromString(FString::Join(All,TEXT("\n")));
}

TArray<FText> UUpgradeManagerComponent::GetCardPreviewLines(const TArray<FAttributePreviewResult>& PreviewResults,
	bool /*bPreferPercent*/,
	int32 MaxDecimals,
	bool bAnnotateClamps)
{
	TArray<FText> Out; Out.Reserve(PreviewResults.Num());
	for (const FAttributePreviewResult& R : PreviewResults)
	{
		EBenefitDirection Dir = (R.ResolvedDirection != EBenefitDirection::InferFromAttribute) ? R.ResolvedDirection : InferDirectionFromTag(R.Attribute);
		const float Delta = R.NewValue - R.OldValue;
		if (R.bUnderlyingChangeMaskedByRounding)
		{
			if (IsPureMultiplicative(R) && !FMath::IsNearlyZero(R.RawOldValue))
			{
				Out.Add(FText::FromString(FormatPercentageChange(R, MaxDecimals)));
			}
			continue;
		}
		if (!FMath::IsNearlyZero(Delta))
		{
			if (IsPureMultiplicative(R) && !FMath::IsNearlyZero(R.OldValue))
			{
				Out.Add(FText::FromString(FormatPercentageChange(R, MaxDecimals)));
			}
			else
			{
				FText Flat=FormatSignedFloat(Delta,MaxDecimals);
				Out.Add(FText::FromString(FString::Printf(TEXT("%s %s"), *GetFriendlyAttributeName(R.Attribute, R.ModifierTextOverride).ToString(), *Flat.ToString())));
			}
		}
		// NEW: Override applied but no visible delta -> still show a line
		else if (R.bHasOverrideChange)
		{
			FNumberFormattingOptions Opts; Opts.MaximumFractionalDigits = MaxDecimals; Opts.MinimumFractionalDigits = 0;
			FText NewValText = FText::AsNumber(R.NewValue, &Opts);
			Out.Add(FText::FromString(
				FString::Printf(TEXT("%s SET TO %s"),
				*GetFriendlyAttributeName(R.Attribute, R.ModifierTextOverride).ToString(),
				*NewValText.ToString())
				));
		}
		else if (R.ClampApplied != EAttributeClampMode::None)
		{
			FString S = GetFriendlyAttributeName(R.Attribute, R.ModifierTextOverride).ToString();
			if (bAnnotateClamps)
			{
				if (R.ClampApplied == EAttributeClampMode::Max) S.Append(TEXT(" (MAX)")); else if (R.ClampApplied==EAttributeClampMode::Min) S.Append(TEXT(" (MIN)"));
			}
			Out.Add(FText::FromString(MoveTemp(S)));
		}
	}
	return Out;
}

void UUpgradeManagerComponent::GetCardPreviewTextsForCard(UObject* WorldContextObject,
	UAttributeComponent* TargetAttributes,
	UUpgradeCardData* Card,
	FText& PositiveText,
	FText& NegativeText,
	FText& CombinedText,
	bool bPreferPercent,
	int32 MaxDecimals,
	bool bAnnotateClamps)
{
	PositiveText=FText();NegativeText=FText();CombinedText=FText();
	if (!WorldContextObject||!TargetAttributes||!Card){return;}
	UUpgradeManagerComponent* Subsys = UUpgradeManagerComponent::Get(WorldContextObject); if(!Subsys)return;
	TArray<FAttributePreviewResult> PreviewResults = Subsys->PreviewCard(TargetAttributes,Card);
	GetCardPreviewTexts(PreviewResults,PositiveText,NegativeText,CombinedText,false,MaxDecimals,bAnnotateClamps);
	// Ability line at top if any
	if (Card->AbilityClass)
	{
		FText AbilityLine = GetCardAbilityDisplayLine(Card);
		if (!AbilityLine.IsEmpty())
		{
			CombinedText = CombinedText.IsEmpty() ? AbilityLine : FText::FromString(AbilityLine.ToString()+TEXT("\n")+CombinedText.ToString());
		}
	}
}

void UUpgradeManagerComponent::GetCardPreviewTextsForCardWithFlags(UObject* WorldContextObject,
	UAttributeComponent* TargetAttributes,
	UUpgradeCardData* Card,
	FText& PositiveText,
	FText& NegativeText,
	FText& CombinedText,
	bool& bHasPositiveChange,
	bool& bHasNegativeChange,
	bool bPreferPercent,
	int32 MaxDecimals,
	bool bAnnotateClamps)
{
	GetCardPreviewTextsForCard(WorldContextObject,TargetAttributes,Card,PositiveText,NegativeText,CombinedText,false,MaxDecimals,bAnnotateClamps);
	bHasPositiveChange=!PositiveText.IsEmpty();
	bHasNegativeChange=!NegativeText.IsEmpty();
}

bool UUpgradeManagerComponent::HasNegativeChange(const TArray<FAttributePreviewResult>& PreviewResults)
{
	for (const FAttributePreviewResult& R : PreviewResults)
	{
		const float Delta = R.NewValue - R.OldValue;
		if (FMath::IsNearlyZero(Delta)) { continue; }
		EBenefitDirection Dir = (R.ResolvedDirection != EBenefitDirection::InferFromAttribute) ? R.ResolvedDirection : InferDirectionFromTag(R.Attribute);
		if (!IsChangeBeneficial(Delta, Dir))
		{
			return true;
		}
	}
	return false;
}

bool UUpgradeManagerComponent::HasNegativeChangeForCard(UObject* WorldContextObject, UAttributeComponent* TargetAttributes, UUpgradeCardData* Card)
{
	if (!WorldContextObject || !TargetAttributes || !Card) { return false; }
	UUpgradeManagerComponent* Subsys = UUpgradeManagerComponent::Get(WorldContextObject); if (!Subsys) return false; 
	const TArray<FAttributePreviewResult> PreviewResults = Subsys->PreviewCard(TargetAttributes, Card);
	for (const FAttributePreviewResult& R : PreviewResults)
	{
		const float Delta = R.NewValue - R.OldValue;
		if (FMath::IsNearlyZero(Delta)) { continue; }
		EBenefitDirection Dir = (R.ResolvedDirection != EBenefitDirection::InferFromAttribute) ? R.ResolvedDirection : InferDirectionFromTag(R.Attribute);
		if (!IsChangeBeneficial(Delta, Dir)) { return true; }
	}
	return false;
}

bool UUpgradeManagerComponent::CardHasNegativeModifierTypes(UUpgradeCardData* Card)
{
	if (!Card)
	{
		return false;
	}
	for (const FAttributeModifierEntry& R : Card->Modifiers)
	{
		EBenefitDirection Dir = (R.BenefitDirection != EBenefitDirection::InferFromAttribute)
			? R.BenefitDirection
			: InferDirectionFromTag(R.TargetAttribute);

		switch (R.Type)
		{
		case EModificationType::Subtraction:
			if (Dir == EBenefitDirection::HigherIsBetter) { return true; }
			break;
		case EModificationType::Multiplication:
			if (Dir == EBenefitDirection::HigherIsBetter)
			{
				if (R.Value < 1.0f) return true;
			}
			else
			{
				if (R.Value > 1.0f) return true;
			}
			break;
		case EModificationType::Addition:
			if (Dir == EBenefitDirection::HigherIsBetter)
			{
				if (R.Value < 0.f) return true;
			}
			else
			{
				if (R.Value > 0.f) return true;
			}
			break;
		case EModificationType::Override:
			break;
		}
	}
	return false;
}

void UUpgradeManagerComponent::GetCardAttributeDecimalPlaces(UObject* WorldContextObject,
	UAttributeComponent* TargetAttributes,
	UUpgradeCardData* Card,
	TArray<FGameplayTag>& AttributeTags,
	TArray<int32>& DecimalPlaces)
{
	AttributeTags.Reset();
	DecimalPlaces.Reset();

	if (!WorldContextObject || !TargetAttributes || !Card)
	{
		return;
	}

	TSet<FGameplayTag> UniqueTags;
	for (const FAttributeModifierEntry& R : Card->Modifiers)
	{
		if (R.TargetAttribute.IsValid())
		{
			UniqueTags.Add(R.TargetAttribute);
		}
	}

	for (const FGameplayTag& Tag : UniqueTags)
	{
		AttributeTags.Add(Tag);
		EAttributeNumericType NumType = TargetAttributes->GetAttributeNumericType(Tag);
		EAttributeRoundingMode Rounding = TargetAttributes->GetAttributeRoundingMode(Tag);
		int32 Places = (NumType == EAttributeNumericType::Integer || Rounding == EAttributeRoundingMode::RoundToNearest) ? 0 : 2;
		DecimalPlaces.Add(Places);
	}
}

FString UUpgradeManagerComponent::FormatPercentageChange(const FAttributePreviewResult& R, int32 MaxDecimals)
{
	const float Pct = ((R.NewValue / R.OldValue) - 1.f) * 100.f;
	const FText PctText = FormatSignedFloat(Pct, FMath::Clamp(MaxDecimals, 0, 2));
	return FString::Printf(TEXT("%s %s%% (×%.2f)"),
		*GetFriendlyAttributeName(R.Attribute, R.ModifierTextOverride).ToString(),
		*PctText.ToString(),
		(R.NewValue / R.OldValue));
}

// Add the missing static accessors (match declarations in the header)
UUpgradeManagerComponent* UUpgradeManagerComponent::Get(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	// Prefer GameState
	if (AGameStateBase* GS = World->GetGameState())
	{
		TArray<UActorComponent*> Comps;
		GS->GetComponents(Comps);
		for (UActorComponent* C : Comps)
		{
			if (UUpgradeManagerComponent* Mgr = Cast<UUpgradeManagerComponent>(C))
			{
				return Mgr;
			}
		}
	}

	// Then GameMode (server)
	if (AGameModeBase* GM = World->GetAuthGameMode<AGameModeBase>())
	{
		TArray<UActorComponent*> Comps;
		GM->GetComponents(Comps);
		for (UActorComponent* C : Comps)
		{
			if (UUpgradeManagerComponent* Mgr = Cast<UUpgradeManagerComponent>(C))
			{
				return Mgr;
			}
		}
	}

	// Fallback: scan any actor
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		TArray<UUpgradeManagerComponent*> Components;
		It->GetComponents<UUpgradeManagerComponent>(Components);
		if (Components.Num() > 0)
		{
			return Components[0];
		}
	}
	return nullptr;
}

UUpgradeManagerComponent* UUpgradeManagerComponent::GetFromAnyWorld()
{
	if (!GEngine) return nullptr;

	for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
	{
		if (UWorld* World = Ctx.World())
		{
			if (UUpgradeManagerComponent* Found = UUpgradeManagerComponent::Get(World))
			{
				return Found;
			}
		}
	}
	return nullptr;
}

// Add the missing selection helpers used by RollUpgrades

bool UUpgradeManagerComponent::CanSelectCardNow(const UUpgradeCardData* Card,
	const UAttributeComponent* Target,
	int32 /*Floor*/,
	int32 PendingCountForCard) const
{
    if (!Card || !Target) return false;

    if (Card->Rules.MaxInstances > 0)
    {
        int32 InstancesFromMods = 0;
        const int32 TotalMods = GetTotalModifierCount(Card);
        if (TotalMods > 0)
        {
            const int32 RefCount = Target->GetAppliedModifierRefCountForSource(const_cast<UUpgradeCardData*>(Card));
            InstancesFromMods = RefCount / FMath::Max(1, TotalMods);
        }
        int32 InstancesFromAbilities = 0;
        if (Card->AbilityClass)
        {
            const AActor* OwnerActor = Target->GetOwner();
            if (OwnerActor)
            {
                if (const UAbilityComponent* AbilityComp = OwnerActor->FindComponentByClass<UAbilityComponent>())
                {
                    if (const FGrantedAbilityList* ListWrapper = GrantedAbilitiesByOwner.Find(const_cast<UAbilityComponent*>(AbilityComp)))
                    {
                        for (const FGrantedAbilityInstance& G : ListWrapper->Abilities)
                        {
                            if (G.SourceCard.Get() == Card)
                            {
                                InstancesFromAbilities++;
                            }
                        }
                    }
                }
            }
        }
        const int32 EffectiveInstances = FMath::Max(InstancesFromMods, InstancesFromAbilities);
        if (EffectiveInstances + PendingCountForCard >= Card->Rules.MaxInstances)
        {
            return false;
        }
    }

    return true;
}

UUpgradeCardData* UUpgradeManagerComponent::PickBonusCardWithFallback(UAttributeComponent* TargetAttributes,
	int32 Floor,
	const TMap<UUpgradeCardData*, int32>& PendingPickCounts) const
{
	if (!TargetAttributes) return nullptr;

	// Helper: weighted pick from a pair array
	auto WeightedPick = [](const TArray<TPair<UUpgradeCardData*, float>>& Arr) -> UUpgradeCardData*
	{
		if (Arr.Num() <= 0) return nullptr;
		float Total = 0.f; for (const auto& P : Arr) { Total += P.Value; }
		if (Total <= 0.f) return Arr.Last().Key;
		const float R = FMath::FRandRange(0.f, Total);
		float Acc = 0.f;
		for (const auto& P : Arr)
		{
			Acc += P.Value;
			if (R <= Acc) { return P.Key; }
		}
		return Arr.Last().Key;
	};

	// Sort rarities by descending sort order (higher rarity first)
	struct FRarityOrder { ERarity R; int32 Sort; };
	TArray<FRarityOrder> Ranks;
	Ranks.Reserve(RarityDataList.Num());
	for (const URarityData* R : RarityDataList)
	{
		if (R) { Ranks.Add({ R->Rarity, R->SortOrder }); }
	}
	Ranks.Sort([](const FRarityOrder& A, const FRarityOrder& B){ return A.Sort > B.Sort; });

	// Identify the two highest rarities and separate the rest
	TArray<ERarity> HighRarities;
	TArray<ERarity> LowerRarities;
	for (const FRarityOrder& Rank : Ranks)
	{
		if (HighRarities.Num() < 2) { HighRarities.Add(Rank.R); }
		else { LowerRarities.Add(Rank.R); }
	}

	// Step 1: Try the two highest rarities, preferring cards not already picked in this roll
	if (HighRarities.Num() >= 2)
	{
		// Build pools for both high rarities
		TArray<TPair<UUpgradeCardData*, float>> HighestAll;  float HighestAllTotal = 0.f;
		TArray<TPair<UUpgradeCardData*, float>> HighestUnused; float HighestUnusedTotal = 0.f;
		TArray<TPair<UUpgradeCardData*, float>> SecondAll;   float SecondAllTotal = 0.f;
		TArray<TPair<UUpgradeCardData*, float>> SecondUnused; float SecondUnusedTotal = 0.f;

		for (UUpgradeCardData* Card : AllUpgradeCards)
		{
			if (!Card) continue;
			if (Card->Rarity != HighRarities[0] && Card->Rarity != HighRarities[1]) continue;
			if (!IsCardValid(Card, TargetAttributes, Floor)) continue;
			const int32 Pending = PendingPickCounts.FindRef(Card);
			if (!CanSelectCardNow(Card, TargetAttributes, Floor, Pending)) continue;
			const float W = GetCardWeight(Card, Floor);
			if (W <= 0.f) continue;

			TPair<UUpgradeCardData*, float> Pair{ Card, W };
			if (Card->Rarity == HighRarities[0])
			{
				HighestAll.Add(Pair); HighestAllTotal += W;
				if (Pending == 0) { HighestUnused.Add(Pair); HighestUnusedTotal += W; }
			}
			else
			{
				SecondAll.Add(Pair); SecondAllTotal += W;
				if (Pending == 0) { SecondUnused.Add(Pair); SecondUnusedTotal += W; }
			}
		}

		const bool bHasHighestUnused = HighestUnused.Num() > 0 && HighestUnusedTotal > 0.f;
		const bool bHasSecondUnused = SecondUnused.Num() > 0 && SecondUnusedTotal > 0.f;
		const bool bHasHighestAll = HighestAll.Num() > 0 && HighestAllTotal > 0.f;
		const bool bHasSecondAll = SecondAll.Num() > 0 && SecondAllTotal > 0.f;

		// If any unused exists across the two top tiers, restrict choice to the tiers that have unused
		if (bHasHighestUnused || bHasSecondUnused)
		{
			bool bChooseHighestTier = bHasHighestUnused && (!bHasSecondUnused || (bHasSecondUnused && FMath::RandBool()));
			const TArray<TPair<UUpgradeCardData*, float>>& ChosenPool = bChooseHighestTier ? HighestUnused : SecondUnused;
			if (UUpgradeCardData* Pick = WeightedPick(ChosenPool))
			{
				if (bLogUpgradeRolls)
				{
					UE_LOG(LogTemp, Log, TEXT("PickBonusCardWithFallback: Chose %s from %s rarity [UNIQUE]"), *Pick->GetName(), bChooseHighestTier ? TEXT("highest") : TEXT("second-highest"));
				}
				return Pick;
			}
		}
		// Otherwise fall back to used pools (duplicates permitted)
		else if (bHasHighestAll || bHasSecondAll)
		{
			bool bChooseHighestTier = bHasHighestAll && (!bHasSecondAll || (bHasSecondAll && FMath::RandBool()));
			const TArray<TPair<UUpgradeCardData*, float>>& ChosenPool = bChooseHighestTier ? HighestAll : SecondAll;
			if (UUpgradeCardData* Pick = WeightedPick(ChosenPool))
			{
				if (bLogUpgradeRolls)
				{
					UE_LOG(LogTemp, Log, TEXT("PickBonusCardWithFallback: Chose %s from %s rarity"), *Pick->GetName(), bChooseHighestTier ? TEXT("highest") : TEXT("second-highest"));
				}
				return Pick;
			}
		}
	}

	// Step 2: Fallback to lower rarities in descending sort order, preferring unused first
	for (ERarity FallbackRarity : LowerRarities)
	{
		TArray<TPair<UUpgradeCardData*, float>> AllPool; float AllTotal = 0.f;
		TArray<TPair<UUpgradeCardData*, float>> UnusedPool; float UnusedTotal = 0.f;

		for (UUpgradeCardData* Card : AllUpgradeCards)
		{
			if (!Card || Card->Rarity != FallbackRarity) continue;
			if (!IsCardValid(Card, TargetAttributes, Floor)) continue;
			const int32 Pending = PendingPickCounts.FindRef(Card);
			if (!CanSelectCardNow(Card, TargetAttributes, Floor, Pending)) continue;
			const float W = GetCardWeight(Card, Floor);
			if (W <= 0.f) continue;

			TPair<UUpgradeCardData*, float> Pair{ Card, W };
			AllPool.Add(Pair); AllTotal += W;
			if (Pending == 0) { UnusedPool.Add(Pair); UnusedTotal += W; }
		}

		if (UnusedPool.Num() > 0 && UnusedTotal > 0.f)
		{
			if (UUpgradeCardData* Pick = WeightedPick(UnusedPool))
			{
				if (bLogUpgradeRolls)
				{
					// Find the sort order for this rarity for logging
					int32 SortOrder = 0; for (const FRarityOrder& Rank : Ranks) { if (Rank.R == FallbackRarity) { SortOrder = Rank.Sort; break; } }
					UE_LOG(LogTemp, Log, TEXT("PickBonusCardWithFallback: Chose %s from fallback rarity (sort %d) [UNIQUE]"), *Pick->GetName(), SortOrder);
				}
				return Pick;
			}
		}
		else if (AllPool.Num() > 0 && AllTotal > 0.f)
		{
			if (UUpgradeCardData* Pick = WeightedPick(AllPool))
			{
				if (bLogUpgradeRolls)
				{
					int32 SortOrder = 0; for (const FRarityOrder& Rank : Ranks) { if (Rank.R == FallbackRarity) { SortOrder = Rank.Sort; break; } }
					UE_LOG(LogTemp, Log, TEXT("PickBonusCardWithFallback: Chose %s from fallback rarity (sort %d)"), *Pick->GetName(), SortOrder);
				}
				return Pick;
			}
		}
	}

	return nullptr;
}
