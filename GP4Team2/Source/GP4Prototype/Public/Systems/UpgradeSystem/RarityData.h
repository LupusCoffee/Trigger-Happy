// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataStructures/AttributeUpgradeDataStructs.h"

#include "RarityData.generated.h"

/**
 * 
 */
UCLASS()
class GP4PROTOTYPE_API URarityData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity|Identifier")
	ERarity Rarity;

	// Name
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity|Identifier")
	FName RarityName;

	// UI Color code
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity|Identifier")
	FLinearColor DisplayColor;

	// To sort in UI in chosen order
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity|Identifier")
	int32 SortOrder = 0;

	// Weight in random rolls
	// TODO: Change Roll weight to FAttribute so modifiers can affect it?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity|Identifier")
	float RollWeight = 1.0f;

	// Value scaling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity|Identifier")
	float ValueMultiplier = 1.0f;

	// FX/Sound
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity|Identifier")
	TObjectPtr<USoundBase> RewardSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity|Identifier")
	TObjectPtr<UMaterialInterface> RarityMaterial;
};
