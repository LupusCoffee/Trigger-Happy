#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AttributeUpgradeDataStructs.generated.h"

class UUpgradeCardData;

UENUM(BlueprintType)
enum class ERarity : uint8
{
	Common		UMETA(DisplayName="Common"),
	Uncommon 	UMETA(DisplayName="Uncommon"),
	Rare 		UMETA(DisplayName="Rare"),
	Epic		UMETA(DisplayName="Epic"),
	Legendary 	UMETA(DisplayName="Legendary"),
};

UENUM(BlueprintType)
enum class EModificationType : uint8
{
	Addition		UMETA(DisplayName="Add (Flat)"),
	Subtraction		UMETA(DisplayName="Subtract (Flat)"),
	Multiplication	UMETA(DisplayName="Multiply (Factor)"),
	Override		UMETA(DisplayName="Override (Set Value)")
};

UENUM(BlueprintType)
enum class EAttributeNumericType : uint8
{
	Float	UMETA(DisplayName="Float"),
	Integer UMETA(DisplayName="Integer")
};

UENUM(BlueprintType)
enum class EAttributeRoundingMode : uint8
{
	None			UMETA(DisplayName="None"),
	RoundToNearest	UMETA(DisplayName="Round To Nearest Int")
};

// New: Clamp mode to allow Min or Max clamping, instead of Max-only
UENUM(BlueprintType)
enum class EAttributeClampMode : uint8
{
	None	UMETA(DisplayName="None"),
	Min		UMETA(DisplayName="Min (Floor)"),
	Max		UMETA(DisplayName="Max (Ceiling)")
};

UENUM(BlueprintType)
enum class EBenefitDirection : uint8
{
	InferFromAttribute = 0 UMETA(DisplayName = "Infer from Attribute"),
	HigherIsBetter     = 1 UMETA(DisplayName = "Higher is Better"),
	LowerIsBetter      = 2 UMETA(DisplayName = "Lower is Better"),
};

USTRUCT(BlueprintType)
struct FAttributeCap
{
	GENERATED_BODY()

	// Attribute to constrain
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(Categories="Attribute", DisplayName="Attribute", ToolTip="Attribute to apply the cap for."))
	FGameplayTag Attribute;

	// Optional Min cap. If bEnforceMin is true, the attribute value after applying a card must be >= MinValue.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Enforce Min", ToolTip="If enabled, the attribute value must be >= Min Value after applying this card."))
	bool bEnforceMin = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Min Value"))
	float MinValue = 0.f;

	// Optional Max cap. If bEnforceMax is true, the attribute value after applying a card must be <= MaxValue.
	// Note: Max enforcement is now owned by the Attribute itself (FAttribute clamp). This per-card setting is deprecated.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Enforce Max (Deprecated)", ToolTip="Deprecated: Max is enforced by the Attribute itself. Leave off and set on the Attribute.", EditConditionHides))
	bool bEnforceMax = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Max Value (Deprecated)", ToolTip="Deprecated: Set max on the Attribute, not per-card.", EditCondition="bEnforceMax", EditConditionHides))
	float MaxValue = 0.f;
};

USTRUCT(BlueprintType)
struct FModifierRule {
	GENERATED_BODY()

	// Floor gating
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Min Floor", ToolTip="Minimum floor for this card to be available. 0 = no minimum."))
	int32 MinFloor = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Max Floor", ToolTip="Maximum floor for this card to be available. 0 = no maximum."))
	int32 MaxFloor = 0;

	// Limit number of times this card can be applied
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Max Instances", ToolTip="Maximum times this exact card can be applied. 0 = unlimited."))
	int32 MaxInstances = 0;

	// Attribute threshold gating
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(Categories="Attribute", DisplayName="Required Attribute", ToolTip="Optional attribute to gate by value."))
	FGameplayTag RequiredAttribute;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Threshold", ToolTip="Threshold value used with Required Attribute."))
	float AttributeThreshold = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Only If Below Threshold", ToolTip="If enabled, card is only valid if attribute is below threshold. If disabled, valid when attribute is >= threshold."))
	bool bOnlyIfBelowThreshold = false;

	// Guarantee appearance rules
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Guarantee Every X Floors", ToolTip="If > 0, guarantee this card appears on floors divisible by this value (e.g., 5 -> 5,10,15...)."))
	int32 GuaranteeEveryXFloors = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Guarantee On Floors", ToolTip="Specific floors where this card is guaranteed to appear."))
	TArray<int32> GuaranteedFloors;

	// Optional per-attribute caps enforced when applying the card
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Attribute Caps"))
	TArray<FAttributeCap> AttributeCaps;
};

USTRUCT(BlueprintType)
struct FAppliedModifierRef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier|Meta")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier|Meta")
	FGuid ModifierID;
};

USTRUCT(BlueprintType)
struct FModifier
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Modifier|Meta", meta=(DisplayName="Modifier ID"))
	FGuid ModifierID;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Modifier|Effect", meta=(DisplayName="Amount / Factor", ToolTip="For Add: flat amount. For Multiply: factor (e.g., 1.1 = +10%). For Override: the exact value."))
	float Value = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Modifier|Effect", meta=(DisplayName="Rarity"))
	ERarity Rarity = ERarity::Common;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Modifier|Effect", meta=(DisplayName="Type"))
	EModificationType Type = EModificationType::Addition;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Upgrade|Rules", meta=(DisplayName="Rules"))
	FModifierRule Rules;
};

USTRUCT(BlueprintType)
struct FAttribute
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attribute", meta=(DisplayName="Base Value"))
	float BaseValue = 0.0f;
	
	UPROPERTY(VisibleAnywhere, Category="Attribute", meta=(DisplayName="Value"))
	float Value = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Attribute", meta=(DisplayName="Active Modifiers"))
	TArray<FModifier> ActiveModifiers;

	// Numeric type (Float vs Integer). If Integer, Value is rounded after recompute.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attribute|Numeric", meta=(DisplayName="Numeric Type"))
	EAttributeNumericType NumericType = EAttributeNumericType::Float;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attribute|Rounding", meta=(DisplayName="Rounding Mode"))
	EAttributeRoundingMode RoundingMode = EAttributeRoundingMode::None;

	// Unified per-attribute clamp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attribute|Clamp", meta=(DisplayName="Clamp Mode"))
	EAttributeClampMode ClampMode = EAttributeClampMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attribute|Clamp", meta=(EditCondition="ClampMode!=EAttributeClampMode::None", DisplayName="Clamp Value"))
	float ClampValue = 0.0f;
	
	void Recalculate()
	{
		Value = BaseValue;

		float AdditiveSum = 0.0f;
		float MultiplicativeFactor = 1.0f;
		TOptional<float> OverrideValue;

		for (const FModifier& M : ActiveModifiers)
		{
			switch (M.Type)
			{
			case EModificationType::Addition:
				AdditiveSum += M.Value;
				break;

			case EModificationType::Subtraction:
				AdditiveSum -= M.Value;
				break;

			case EModificationType::Multiplication:
				MultiplicativeFactor *= M.Value;
				break;

			case EModificationType::Override:
				OverrideValue = M.Value;
				break;
			}
		}

		Value = (Value + AdditiveSum) * MultiplicativeFactor;

		if (OverrideValue.IsSet())
		{
			Value = OverrideValue.GetValue();
		}

		// If the attribute is declared as Integer, coerce using the rounding policy first
		if (NumericType == EAttributeNumericType::Integer)
		{
			switch (RoundingMode)
			{
			case EAttributeRoundingMode::RoundToNearest:
				Value = static_cast<float>(FMath::RoundToInt(Value));
				break;
			case EAttributeRoundingMode::None:
			default:
				// Default to nearest if integer type but no explicit rounding set
				Value = static_cast<float>(FMath::RoundToInt(Value));
				break;
			}
		}

		// Finally, enforce attribute-level clamp
		switch (ClampMode)
		{
		case EAttributeClampMode::Max:
			Value = FMath::Min(Value, ClampValue);
			break;
		case EAttributeClampMode::Min:
			Value = FMath::Max(Value, ClampValue);
			break;
		case EAttributeClampMode::None:
		default:
			break;
		}
	}
};

USTRUCT(BlueprintType)
struct GP4PROTOTYPE_API FAttributeModifierEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier|Id", meta=(DisplayName="Id"))
	FGuid Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier|Target", meta=(Categories="Attribute", DisplayName="Attribute"))
	FGameplayTag TargetAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier|Effect", meta=(DisplayName="Type"))
	EModificationType Type = EModificationType::Multiplication;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier|Effect", meta=(DisplayName="Amount / Factor"))
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modifier|Display", meta=(DisplayName="Benefit Direction"))
	EBenefitDirection BenefitDirection = EBenefitDirection::InferFromAttribute;

	void EnsureId()
	{
		if (!Id.IsValid())
		{
			Id = FGuid::NewGuid();
		}
	}
};

USTRUCT(BlueprintType)
struct FAttributePreviewResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	FGameplayTag Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	FText ModifierTextOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	float OldValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	float NewValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	float Delta = 0.f;

	// Original flag (kept for compatibility). True if attribute-level clamp reduced/increased the projected value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	bool bClampedByAttributeMax = false;

	// New: which clamp was applied during preview (None/Min/Max)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	EAttributeClampMode ClampApplied = EAttributeClampMode::None;

	// True if a per-card Min cap would be violated for this tag (used in card preview)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	bool bViolatesMinCap = false;

	// NEW: raw (pre-rounding, pre-clamp) values to allow UI to show meaningful percentage even if rounding masks change
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	float RawOldValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	float RawNewValue = 0.f;

	// NEW: true if RawNewValue differs from RawOldValue but rounding/clamp produced zero visible Delta
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	bool bUnderlyingChangeMaskedByRounding = false;

	// NEW: metadata about how the preview delta was produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	bool bHasAdditiveChange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	bool bHasMultiplicativeChange = false;

	// NEW: true if an Override-type modifier exists among the NEW modifiers being previewed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	bool bHasOverrideChange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	float NetAdditiveDelta = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	float NetMultiplicativeFactor = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	EBenefitDirection ResolvedDirection = EBenefitDirection::InferFromAttribute;
};
