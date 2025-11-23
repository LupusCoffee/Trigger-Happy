// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RarityData.h"
#include "UpgradeCardData.h"
#include "DataStructures/AttributeUpgradeDataStructs.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "Curves/CurveFloat.h"
#include "Systems/CombatSystem/Abilities/GameplayAbilityObject.h"
#include "Components/ActorComponent.h"

#include "UpgradeManagerComponent.generated.h"

USTRUCT()
struct FGrantedAbilityInstance
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UUpgradeCardData> SourceCard;

	UPROPERTY()
	TObjectPtr<UGameplayAbilityObject> Instance = nullptr;

	UPROPERTY()
	ECardAbilityType Type = ECardAbilityType::Passive;
};

USTRUCT()
struct FGrantedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGrantedAbilityInstance> Abilities;
};

UCLASS(ClassGroup="(Attribute)", meta=(BlueprintSpawnableComponent))
class GP4PROTOTYPE_API UUpgradeManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Author‑assigned pool of all upgrade cards (set in editor / data setup)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade")
	TArray<TObjectPtr<UUpgradeCardData>> AllUpgradeCards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade")
	TArray<TObjectPtr<URarityData>> RarityDataList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade|Rarity")
	TObjectPtr<UCurveFloat> RarityBiasCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade|Level")
	AActor* LevelManager = nullptr;

	// Debug helpers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade|Debug") bool bUseDebugFloorOverride = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade|Debug", meta=(ClampMin="1")) int32 DebugFloorOverride = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade|Debug") bool bLogUpgradeRolls = false;

	// Bonus rarity handling
	UFUNCTION(BlueprintCallable, Category="Upgrade|Bonus") void RequestHighRarityBonusNextRoll(UObject* Context);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade|Bonus") bool HasPendingHighRarityBonusFor(UObject* Context) const;

	// Convenience: request using an AttributeComponent; will internally resolve to the same context RollUpgrades uses
	UFUNCTION(BlueprintCallable, Category="Upgrade|Bonus") void RequestHighRarityBonusNextRollForAttributes(UAttributeComponent* TargetAttributes);

	UFUNCTION(BlueprintCallable, Category="Upgrade") void SetLevelManager(AActor* InManager) { LevelManager = InManager; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade") int32 GetCurrentFloor() const;

	UFUNCTION(BlueprintCallable, Category="Upgrade") TArray<UUpgradeCardData*> RollUpgrades(UAttributeComponent* TargetAttributes, int32 NumCards);
	UFUNCTION(BlueprintCallable, Category="Upgrade") ERarity RollRarity(int Difficulty);
	UFUNCTION(BlueprintCallable, Category="Upgrade") TArray<FModifier> GetValidModifiers(ERarity Rarity);
	UFUNCTION(BlueprintCallable, Category="Upgrade") FModifier PickRandomModifier(const TArray<FModifier>& Candidates);
	UFUNCTION(BlueprintCallable, Category="Upgrade") URarityData* GetRarityData(ERarity Rarity) const;
	UFUNCTION(BlueprintCallable, Category="Upgrade") void ApplyCardToAttributes(UAttributeComponent* AttributeComp, UUpgradeCardData* Card) const;
	UFUNCTION(BlueprintCallable, Category="Upgrade") void RemoveCardFromAttributes(UAttributeComponent* AttributeComp, UUpgradeCardData* Card) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade|Debug") TArray<UUpgradeCardData*> GetGuaranteedCardsForFloor(UAttributeComponent* TargetAttributes, int32 Floor) const;
	UFUNCTION(BlueprintCallable, Category="Upgrade|Preview") EBenefitDirection ResolveDirectionForTag(FGameplayTag Tag, TArray<FAttributeModifierEntry> AttributeModifierEntries) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade|Preview") TArray<FAttributePreviewResult> PreviewCard(UAttributeComponent* TargetAttributes, UUpgradeCardData* Card) const;

	// Ability queries (leave ability system intact)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade|Ability") UGameplayAbilityObject* GetActiveAbilityForOwner(UObject* OwnerContext) const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Upgrade|Ability") TArray<UGameplayAbilityObject*> GetPassiveAbilitiesForOwner(UObject* OwnerContext) const;

	// Used to live in CardPreviewHelper;
	
	// Create a human-friendly display name from an Attribute tag (e.g., "Attribute.MaxHealth" -> "Max Health").
	UFUNCTION(BlueprintPure, Category="Upgrade|Preview")
	static FText GetFriendlyAttributeName(FGameplayTag AttributeTag, const FText& NameOverride);

	// Format a single attribute delta as text, choosing percent or absolute formatting.
	// Example outputs: "Max Health +2%", "Dash Cooldown -0.5s", "Sprint Speed +30".
	UFUNCTION(BlueprintPure, Category="Upgrade|Preview")
	static FText FormatAttributeChange(FGameplayTag AttributeTag,
	                                   float OldValue,
	                                   float NewValue,
	                                   bool bPreferPercent = true,
	                                   int32 MaxDecimals = 0,
	                                   bool bAnnotateClamps = true,
	                                   bool bClampedByAttributeMax = false,
	                                   const FText& NameOverride = FText());

	// Build ready-to-use texts from a Card Preview without needing a ForEach in BP.
	// - PositiveText: the largest positive change line (empty if none)
	// - NegativeText: the largest negative change line (empty if none)
	// - CombinedText: newline-joined list of all changes in sign order (positives, then negatives)
	UFUNCTION(BlueprintPure, Category="Upgrade|Preview")
	static void GetCardPreviewTexts(const TArray<FAttributePreviewResult>& PreviewResults,
    FText& PositiveText,
    FText& NegativeText,
    FText& CombinedText,
    bool bPreferPercent,
    int32 MaxDecimals,
    bool bAnnotateClamps);

	// Optional: returns all formatted lines as an array if you need them.
	UFUNCTION(BlueprintPure, Category="Upgrade|Preview")
	static TArray<FText> GetCardPreviewLines(const TArray<FAttributePreviewResult>& PreviewResults,
	                                         bool bPreferPercent = true,
	                                         int32 MaxDecimals = 0,
	                                         bool bAnnotateClamps = true);

	// Convenience: do the preview internally via UpgradeManagerSubsystem and return texts directly.
	UFUNCTION(BlueprintPure, meta=(WorldContext="WorldContextObject"), Category="Upgrade|Preview")
	static void GetCardPreviewTextsForCard(UObject* WorldContextObject,
    UAttributeComponent* TargetAttributes,
    UUpgradeCardData* Card,
    FText& PositiveText,
    FText& NegativeText,
    FText& CombinedText,
    bool bPreferPercent = true,
    int32 MaxDecimals = 0,
    bool bAnnotateClamps = true);

	// Same as above, but also tells you if there are any positive/negative changes present in the preview.
	UFUNCTION(BlueprintPure, meta=(WorldContext="WorldContextObject"), Category="Upgrade|Preview")
	static void GetCardPreviewTextsForCardWithFlags(UObject* WorldContextObject,
	                                                UAttributeComponent* TargetAttributes,
	                                                UUpgradeCardData* Card,
	                                                FText& PositiveText,
	                                                FText& NegativeText,
	                                                FText& CombinedText,
	                                                bool& bHasPositiveChange,
	                                                bool& bHasNegativeChange,
	                                                bool bPreferPercent = true,
	                                                int32 MaxDecimals = 0,
	                                                bool bAnnotateClamps = true);

	// Quick predicate: true if any previewed attribute change is negative (New < Old).
	UFUNCTION(BlueprintPure, Category="Upgrade|Preview")
	static bool HasNegativeChange(const TArray<FAttributePreviewResult>& PreviewResults);

	// Quick predicate: preview the card and tell if any change is negative.
	UFUNCTION(BlueprintPure, meta=(WorldContext="WorldContextObject"), Category="Upgrade|Preview")
	static bool HasNegativeChangeForCard(UObject* WorldContextObject, UAttributeComponent* TargetAttributes,
	                                     UUpgradeCardData* Card);

	// Asset-level predicate: does the card contain any negative-leaning modifier types (Subtract or Multiply < 1.0, or Add with negative value)?
	UFUNCTION(BlueprintPure, Category="Upgrade|Preview")
	static bool CardHasNegativeModifierTypes(UUpgradeCardData* Card);

	// NEW: For a given card, return per-attribute decimal places recommended for UI display.
	// Integer attributes -> 0 decimals. Float attributes -> up to 2 decimals.
	// Outputs aligned arrays so they are easy to use in Blueprints (AttributeTags[i] -> DecimalPlaces[i]).
	UFUNCTION(BlueprintPure, meta=(WorldContext="WorldContextObject"), Category="Upgrade|Preview")
	static void GetCardAttributeDecimalPlaces(UObject* WorldContextObject,
		UAttributeComponent* TargetAttributes,
		UUpgradeCardData* Card,
		TArray<FGameplayTag>& AttributeTags,
		TArray<int32>& DecimalPlaces);
	static FString FormatPercentageChange(const FAttributePreviewResult& R, int32 MaxDecimals);

	// Ability preview: returns user-facing ability name line (e.g., "Active Ability: Phase Dash") or empty if none.
	UFUNCTION(BlueprintPure, Category="Upgrade|Preview")
	static FText GetCardAbilityDisplayLine(UUpgradeCardData* Card);

	// Ability preview: returns display name only (without prefix), or empty if no ability.
	UFUNCTION(BlueprintPure, Category="Upgrade|Preview")
	static FText GetCardAbilityDisplayName(UUpgradeCardData* Card);

	// Convenience accessor to find the first UpgradeManagerComponent in the current world (GameState, GameMode, else any actor).
	UFUNCTION(BlueprintPure, meta=(WorldContext="WorldContextObject"), Category="Upgrade")
	static UUpgradeManagerComponent* Get(UObject* WorldContextObject);
	// Editor-friendly: find a manager component from any loaded world (PIE/Editor/Game). Returns first found or nullptr.
	UFUNCTION(BlueprintPure, Category="Upgrade")
	static UUpgradeManagerComponent* GetFromAnyWorld();

	// UI helpers for ability cards
	UFUNCTION(BlueprintPure, Category="Upgrade|Ability")
	static bool CardIsAbilityCard(UUpgradeCardData* Card) { return Card && Card->AbilityClass != nullptr; }

	UFUNCTION(BlueprintCallable, Category="Upgrade|Ability")
	void ApplyAbilitiesFromCard(UAbilityComponent* AbilityComp, UUpgradeCardData* Card) const;
	UFUNCTION(BlueprintCallable, Category="Upgrade|Ability")
	void RemoveAbilitiesFromCard(UAbilityComponent* AbilityComp, UUpgradeCardData* Card) const;

private:
	float GetRarityMultiplier(ERarity InRarity) const;
	bool IsCardValid(const UUpgradeCardData* Card, const UAttributeComponent* Target, int32 Floor) const;
	float GetCardWeight(const UUpgradeCardData* Card, int32 Floor) const;
	bool WouldRespectCapsApprox(const UUpgradeCardData* Card, const UAttributeComponent* Target) const;
	bool EnforceCapsAndApplyForTag(UAttributeComponent* AttributeComp,
		UUpgradeCardData* Card,
		FGameplayTag Tag,
		const TArray<FAttributeModifierEntry>& ModsForTag,
		float Scale) const;
	bool IsCardGuaranteedOnFloor(const UUpgradeCardData* Card, int32 Floor) const;
	bool IsCardValidIgnoringFloor(const UUpgradeCardData* Card, const UAttributeComponent* Target) const;
	int32 GetTotalModifierCount(const UUpgradeCardData* Card) const;

	// Bonus tracking
	UPROPERTY() TSet<TWeakObjectPtr<UObject>> PendingHighRarityBonus;
	UPROPERTY() int32 PendingBonusRollCount = 0; // Global bonus roll counter
	bool ConsumePendingBonusFor(UObject* Context);
	bool CanSelectCardNow(const UUpgradeCardData* Card, const UAttributeComponent* Target, int32 Floor, int32 PendingCountForCard) const;
	UUpgradeCardData* PickBonusCardWithFallback(UAttributeComponent* TargetAttributes, int32 Floor, const TMap<UUpgradeCardData*, int32>& PendingPickCounts) const;

	// Ability management
	mutable TMap<TWeakObjectPtr<UObject>, FGrantedAbilityList> GrantedAbilitiesByOwner;
	void GrantAbilityForCard(UAbilityComponent* AbilityComp, UUpgradeCardData* Card) const;
	void RevokeAbilitiesFromCard(UObject* OwnerContext, UUpgradeCardData* Card) const;
	void RevokeAllAbilitiesForOwner(UObject* OwnerContext) const;
	void RevokeAllAbilities() const;
};
