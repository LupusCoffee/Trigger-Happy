#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AgentData.h"
#include "GameplayTagContainer.h"
#include "DataStructures/AttributeUpgradeDataStructs.h"
#include "TimerManager.h"
#include "AttributeComponent.generated.h"

class UUpgradeCardData;

UCLASS(ClassGroup="(Attribute)", meta=(BlueprintSpawnableComponent))
class GP4PROTOTYPE_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

	virtual void InitializeComponent() override; 
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

public:
	UAttributeComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	bool bEnableAttributeDebugLogging = false;

	// Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeValueChangedSignature, FGameplayTag, AttributeTag, float,
	                                               OldValue, float, NewValue);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeModifiedSignature, FGameplayTag, AttributeTag, FModifier,
	                                             Modifier);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAnyAttributeChangedSignature, FGameplayTag, AttributeTag, float,
	                                               OldValue, float, NewValue);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeBaseChangedSignature, FGameplayTag, AttributeTag, float,
	                                               OldBase, float, NewBase);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttributeInitializedSignature);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Attribute|Data")
	TMap<FGameplayTag, FAttribute> Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attribute|Data")
	UAgentData* AgentData;

	// -- Attributes --
#pragma region Attributes
	FAttribute& MaxHealth = RegisterAndGetAttribute("Attribute.MaxHealth", "Base MaxHealth attribute");
	FAttribute& MaxDefense = RegisterAndGetAttribute("Attribute.MaxDefense", "Base MaxDefense attribute");
	
	FAttribute& DashCooldown = RegisterAndGetAttribute("Attribute.Dash.Cooldown", "Cooldown of the dash in seconds");
	FAttribute& DashCooldownPerCharge = RegisterAndGetAttribute("Attribute.Dash.CooldownPerCharge", "Cooldown of the dash per charge in seconds");
	FAttribute& DashKnockbackForce = RegisterAndGetAttribute("Attribute.Dash.KnockbackForce", "Force applied to the target on dash collision");
	FAttribute& DashKnockbackForceDead = RegisterAndGetAttribute("Attribute.Dash.KnockbackForceDead", "Force applied to the dead target on dash collision");
	FAttribute& DashStrength = RegisterAndGetAttribute("Attribute.Dash.Strength", "Strength of the dash impulse");
	FAttribute& DashMaxCharges = RegisterAndGetAttribute("Attribute.Dash.MaxCharges", "Maximum number of dash charges");
	FAttribute& DashCollisionDamage = RegisterAndGetAttribute("Attribute.Dash.CollisionDamage", "Damage dealt when colliding with an enemy during a dash");

	FAttribute& KillstreakExplosiveRounds = RegisterAndGetAttribute("Attribute.Killstreak.ExplosiveRounds", "Number of explosive rounds granted on killstreak");

	FAttribute& WalkSpeed = RegisterAndGetAttribute("Attribute.Movement.WalkSpeed", "Walking speed");
	FAttribute& SprintSpeed = RegisterAndGetAttribute("Attribute.Movement.SprintSpeed", "Sprinting speed");
	FAttribute& CrouchSpeed = RegisterAndGetAttribute("Attribute.Movement.CrouchSpeed", "Crouching speed");

	FAttribute& MeleeDamage = RegisterAndGetAttribute("Attribute.Melee.Damage", "Damage dealt by melee attacks");
	FAttribute& MeleeKnockbackForce = RegisterAndGetAttribute("Attribute.Melee.KnockbackForce", "Force applied to the target on hit");
	FAttribute& MeleeKnockbackForceDead = RegisterAndGetAttribute("Attribute.Melee.KnockbackForceDead", "Force applied to the dead target on hit");
	FAttribute& MeleeCooldown = RegisterAndGetAttribute("Attribute.Melee.Cooldown", "Cooldown between melee attacks");
	FAttribute& MeleeHitDetection = RegisterAndGetAttribute("Attribute.Melee.HitDetection", "Amount of enemies that can be hit per melee attack");
	FAttribute& MeleeHitDetectionRadius = RegisterAndGetAttribute("Attribute.Melee.HitDetectionRadius", "Radius of the melee hit detection sphere");
	FAttribute& MeleeSwingAmount = RegisterAndGetAttribute("Attribute.Melee.SwingAmount", "Degrees to swing the melee weapon");

	FAttribute& SlideStrength = RegisterAndGetAttribute("Attribute.Slide.Strength", "Strength of the slide impulse");
	FAttribute& SlideDuration = RegisterAndGetAttribute("Attribute.Slide.Duration", "Duration of the slide in seconds");
	FAttribute& SlideCooldown = RegisterAndGetAttribute("Attribute.Slide.Cooldown", "Cooldown of the slide in seconds");
	FAttribute& SlideCollisionDamage = RegisterAndGetAttribute("Attribute.Slide.CollisionDamage", "Damage dealt when colliding with an enemy during a slide");

	FAttribute& SlowMoTimeDilation = RegisterAndGetAttribute("Attribute.SlowMo.TimeDilation", "Global time dilation factor when slow-mo is active (e.g., 0.2..0.5)");
	FAttribute& SlowMoMaxDuration = RegisterAndGetAttribute("Attribute.SlowMo.MaxDuration", "Maximum duration of slow-mo when triggered (seconds)");
	FAttribute& SlowMoCooldown = RegisterAndGetAttribute("Attribute.SlowMo.Cooldown", "Cooldown between slow-mo activations (seconds)");

	FAttribute& ThrowForce = RegisterAndGetAttribute("Attribute.Throw.Force", "Force applied to thrown objects");
	FAttribute& ThrowDamage = RegisterAndGetAttribute("Attribute.Throw.Damage", "Damage dealt by thrown objects");

	FAttribute& FireDelay = RegisterAndGetAttribute("Attribute.Weapon.FireDelay", "Delay between shots");
	FAttribute& ReloadSpeed = RegisterAndGetAttribute("Attribute.Weapon.ReloadSpeed", "Speed of reloading");
	FAttribute& WeaponDamage = RegisterAndGetAttribute("Attribute.Weapon.Damage", "Base weapon damage");
#pragma endregion

	// -- Attribute|Get --
#pragma region Get
	UFUNCTION(BlueprintPure, Category="Attribute|Get")
	FAttribute GetAttribute(FGameplayTag tag);

	UFUNCTION(BlueprintPure, Category="Attribute|Get")
	FGameplayTag GetAttributeTag(FAttribute attribute);

	UFUNCTION(BlueprintPure, Category="Attribute|Get")
	float GetAttributeBaseValue(FGameplayTag tag) const;

	UFUNCTION(BlueprintPure, Category="Attribute|Get")
	float GetAttributeValue(FGameplayTag tag) const;

	// Convenience: get as int (rounded), regardless of numeric type
	UFUNCTION(BlueprintPure, Category="Attribute|Get")
	int32 GetAttributeValueInt(FGameplayTag Tag) const;

	UFUNCTION(BlueprintPure, Category="Attribute|Get")
	float GetAttributeClamped(FGameplayTag Tag, float Min, float Max) const;

	UFUNCTION(BlueprintPure, Category="Attribute|Get")
	bool HasAttribute(FGameplayTag Tag) const;

	UFUNCTION(BlueprintPure, Category="Attribute|Get")
	bool HasModifier(FGameplayTag Tag, FGuid ID) const;

	UFUNCTION(BlueprintPure, Category="Attribute|Get")
	int32 GetAppliedModifierRefCountForSource(UObject* Source) const;

	// Numeric type
	UFUNCTION(BlueprintCallable, Category="Attribute|Numeric")
	void SetAttributeNumericType(FGameplayTag Tag, EAttributeNumericType Type);

	UFUNCTION(BlueprintPure, Category="Attribute|Numeric")
	EAttributeNumericType GetAttributeNumericType(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category="Attribute|Rounding")
	void SetAttributeRoundingMode(FGameplayTag Tag, EAttributeRoundingMode Mode);

	UFUNCTION(BlueprintPure, Category="Attribute|Rounding")
	EAttributeRoundingMode GetAttributeRoundingMode(FGameplayTag Tag) const;

	// Attribute-level Clamp getters (compat wrappers for legacy Max-only API)
	UFUNCTION(BlueprintPure, Category="Attribute|Clamp")
	bool IsAttributeUsingMaxClamp(FGameplayTag Tag) const { if (const FAttribute* A = Attributes.Find(Tag)) return A->ClampMode == EAttributeClampMode::Max; return false; }
	UFUNCTION(BlueprintPure, Category="Attribute|Clamp")
	float GetAttributeMaxClampValue(FGameplayTag Tag) const { const FAttribute* A = Attributes.Find(Tag); if (A && A->ClampMode == EAttributeClampMode::Max) return A->ClampValue; return 0.f; }
	// Optional: direct accessors (not exposed before)
	EAttributeClampMode GetAttributeClampMode(FGameplayTag Tag) const { if (const FAttribute* A = Attributes.Find(Tag)) return A->ClampMode; return EAttributeClampMode::None; }
	float GetAttributeClampValue(FGameplayTag Tag) const { if (const FAttribute* A = Attributes.Find(Tag)) return A->ClampValue; return 0.f; }
#pragma endregion

	// -- Attribute|Set --
#pragma region Set
	UFUNCTION(BlueprintCallable, Category="Attribute|Set")
	void SetAttributeBaseValue(FGameplayTag tag, float NewValue);

	UFUNCTION(BlueprintCallable, Category="Attribute|Set")
	void SetAttributeValue(FGameplayTag tag, float NewValue);

	UFUNCTION(BlueprintCallable, Category="Attribute|Set")
	void ResetAttribute(FGameplayTag Tag);
#pragma endregion

	// -- Attribute|Modifier --
#pragma region Modifier
	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	void AddModifier(FGameplayTag Tag, const FModifier& Modifier);

	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	void AddModifiers(FGameplayTag Tag, const TArray<FModifier>& Modifiers);

	// Convenience: add and register a modifier from a source object, returning the assigned ModifierID
	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	FGuid AddModifierFromSource(FGameplayTag Tag, const FModifier& Modifier, UObject* Source);

	// Add a temporary modifier that will auto-remove after DurationSeconds. Returns the ModifierID for early cancel.
	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	FGuid AddTemporaryModifier(FGameplayTag Tag, const FModifier& Modifier, float DurationSeconds, UObject* Source);

	// Remove a specific modifier by its ID on the given tag. Returns true if removed.
	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	bool RemoveModifierByID(FGameplayTag Tag, FGuid ModifierID);

	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	void RemoveModifier(FGameplayTag Tag, const FModifier& Modifier);

	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	void ClearModifiers(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	void RegisterAppliedModifier(UObject* Source, FGameplayTag Tag, FGuid ModifierID);

	// Remove one registered ref mapping when a modifier is explicitly removed/cancelled
	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	void UnregisterAppliedModifier(UObject* Source, FGuid ModifierID);

	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	void RemoveAllModifiersFromSourceObject(UObject* Source);

	// Cancel a temporary modifier early by its ID (clears timer and removes modifier if still present)
	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	bool CancelTemporaryModifier(FGuid ModifierID);

	// Cancel all temporary modifiers associated with a source (does not wait for duration)
	UFUNCTION(BlueprintCallable, Category="Attribute|Modifier")
	void CancelAllTemporaryModifiersFromSource(UObject* Source);
#pragma endregion

	// -- Attribute|Recalculate --
#pragma region Recalculate
	UFUNCTION(BlueprintCallable, Category="Attribute|Recalculate")
	void RecalculateAttribute(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category="Attribute|Recalculate")
	void RecalculateAll();
#pragma endregion

	// -- Attribute|Load --
#pragma region Load
	UFUNCTION(BlueprintCallable, Category="Attribute|Load")
	void InitializeFromAgentData(UAgentData* newAgentData);

	UFUNCTION(BlueprintCallable, Category="Attribute|Load")
	FAttribute& RegisterAndGetAttribute(const FString& TagName, const FString& DevComment);

	UFUNCTION(BlueprintPure, Category="Attribute|State")
	bool IsInitialized() const { return bAttributesInitialized; }
	
#pragma endregion

	// -- Events --
#pragma region Events
	// OnAttributeValueChanged
	UPROPERTY(BlueprintAssignable, Category="Attribute|Event")
	FOnAttributeValueChangedSignature OnAttributeValueChanged;

	// OnAnyAttributeChanged
	UPROPERTY(BlueprintAssignable, Category="Attribute|Event")
	FOnAnyAttributeChangedSignature OnAnyAttributeChanged;

	// OnAttributeBaseChanged
	UPROPERTY(BlueprintAssignable, Category="Attribute|Event")
	FOnAttributeBaseChangedSignature OnAttributeBaseChanged;

	// OnAttributeModified
	UPROPERTY(BlueprintAssignable, Category="Attribute|Event")
	FOnAttributeModifiedSignature OnAttributeModified;

	// OnAttributeInitialized
	UPROPERTY(BlueprintAssignable, Category="Attribute|Event")
	FOnAttributeInitializedSignature OnAttributeInitialized;
#pragma endregion

	// -- Attribute|Preview --
#pragma region Preview
	UFUNCTION(BlueprintPure, Category="Attribute|Preview")
	FAttributePreviewResult PreviewApplyModifierForTag(FGameplayTag Tag, const FModifier& NewModifier) const;

	UFUNCTION(BlueprintPure, Category="Attribute|Preview")
	FAttributePreviewResult PreviewApplyModifiersForTag(FGameplayTag Tag, const TArray<FModifier>& NewModifiers) const;

	// Debug
	UFUNCTION(BlueprintCallable, Category="Attribute|Debug")
	void DumpAttributes() const;
#pragma endregion

private:
	TMap<TWeakObjectPtr<UObject>, TArray<FAppliedModifierRef>> SourceAppliedModifiers;

	TMap<FGuid, FGameplayTag> TempIdToTag;
	TMap<FGuid, FTimerHandle> TempRemovalTimers;

	bool bAttributesInitialized = false;
	void EnsureAttributesInitialized();

	void HandleTempModifierExpired(FGuid ModifierID);
};
