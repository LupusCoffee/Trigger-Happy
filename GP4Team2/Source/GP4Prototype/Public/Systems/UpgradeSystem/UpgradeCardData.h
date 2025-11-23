// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataStructures/AttributeUpgradeDataStructs.h"
#include "Engine/DataAsset.h"
#include "Systems/CombatSystem/Abilities/GameplayAbilityObject.h"
#include "UpgradeCardData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GP4PROTOTYPE_API UUpgradeCardData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Display",
		meta=(DisplayName="Name", ToolTip="Name shown to the player."))
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Display",
		meta=(DisplayName="Description", MultiLine=true, ToolTip="Short description of what this upgrade does."))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Display",
		meta=(DisplayName="Icon", ToolTip="Card icon for UI."))
	UTexture2D* Icon;

	// Modifier Text override without touching the value (i.e "Max Health" -> "Health" or "HP")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifiers|Attribute",
		meta=(DisplayName="Modifier Text Override", ToolTip="Optional override for modifier attribute names."))
	FText ModifierTextOverride;

	// Inline modifiers (now the only source)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifiers|Attribute",
		meta=(DisplayName="Modifiers", ToolTip="Per-card modifiers"))
	TArray<FAttributeModifierEntry> Modifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Rarity",
		meta=(DisplayName="Rarity", ToolTip="Rarity class determines roll weight and value scaling."))
	ERarity Rarity;

	// Optional gameplay ability granted when this card is applied
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifiers|Ability", meta=(DisplayName="Ability Class"))
	TSubclassOf<UGameplayAbilityObject> AbilityClass;

	// Type of the granted ability (Passive = many, Active = replaces existing active ability)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifiers|Ability", meta=(EditCondition="AbilityClass!=nullptr"))
	ECardAbilityType AbilityType = ECardAbilityType::Passive;

	// Optional override display name/description (fallback to ability defaults if empty)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifiers|Ability", meta=(EditCondition="AbilityClass!=nullptr"))
	FText AbilityDisplayNameOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifiers|Ability", meta=(EditCondition="AbilityClass!=nullptr", MultiLine=true))
	FText AbilityDescriptionOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="",
		meta=(DisplayName="Rules", ToolTip="Availability limits, thresholds, caps and guarantees for this card."))
	FModifierRule Rules;
};
