// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DataStructures/AttributeUpgradeDataStructs.h"
#include "AgentData.generated.h"

USTRUCT(BlueprintType, Category="Attribute", meta=(DisplayName="Agent Attribute"))
struct FAgentAttribute
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Attribute|Identifier", meta=(Categories="Attribute"))
	FGameplayTag AttributeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Attribute|Identifier")
	float BaseValue = 0.f;

	// Numeric type for this attribute (float or integer)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Attribute|Identifier")
	EAttributeNumericType NumericType = EAttributeNumericType::Float;

	// Optional rounding policy for this attribute (applied at runtime)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Attribute|Identifier")
	EAttributeRoundingMode RoundingMode = EAttributeRoundingMode::None;

	// Unified Clamp (None/Min/Max)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Attribute|Clamp", meta=(DisplayName="Clamp Mode"))
	EAttributeClampMode ClampMode = EAttributeClampMode::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Attribute|Clamp", meta=(EditCondition="ClampMode!=EAttributeClampMode::None", DisplayName="Clamp Value"))
	float ClampValue = 0.f;
};

/**
 * 
 */
UCLASS(BlueprintType)
class GP4PROTOTYPE_API UAgentData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	TArray<FAgentAttribute> Attributes;
};
