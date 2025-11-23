#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayAbilityObject.generated.h"
class UAbilityComponent;


// Active = only one may be active per owner at a time. Passive = any number may coexist.
UENUM(BlueprintType)
enum class ECardAbilityType : uint8
{
	Passive UMETA(DisplayName="Passive"),
	Active  UMETA(DisplayName="Active")
};


/**
 * Lightweight, extensible gameplay ability base (NOT GAS). Intended to be granted by Upgrade Cards.
 * Lifecycle: Instantiate (NewObject) -> Activate(Context) -> (optionally) Deactivate() when card removed / replaced.
 */
UCLASS(Blueprintable, Abstract, EditInlineNew, DefaultToInstanced)
class GP4PROTOTYPE_API UGameplayAbilityObject : public UObject
{
	GENERATED_BODY()
	
public:
	// Display data (optional; card DisplayName can override in UI)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Display")
	FText AbilityName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Display", meta=(MultiLine=true))
	FText AbilityDescription;

	// Default type hint (cards also store the intended type; card wins if mismatch)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	ECardAbilityType DefaultAbilityType = ECardAbilityType::Passive;

	// The duration of the ability, if needed.
	UPROPERTY(EditDefaultsOnly, Category="Ability|State")
	float AbilityDuration = false;
	
	// The cooldown of the ability after use, if needed.
	UPROPERTY(EditDefaultsOnly, Category="Ability|State")
	float AbilityCooldown = false;

	UPROPERTY(EditDefaultsOnly, Category="Ability|State")
	float AdditionalCooldownOnAdjustedCooldown = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category="Ability|State")
	bool MustHoldDownInputThroughout = false;

	UPROPERTY(EditDefaultsOnly, Category="Ability|State")
	bool AdjustCooldownBasedOnTickedTime = false;


	// True while Activate succeeded and before Deactivate
	UPROPERTY(BlueprintReadOnly, Category="Ability|State")
	bool bIsActive = false;

	// The context object passed on activation (owning component/actor etc.)
	UPROPERTY(BlueprintReadOnly, Category="Ability|State")
	TObjectPtr<UObject> ContextObject;

	// How far along the duration we are
	UPROPERTY(BlueprintReadOnly, Category="Ability|State")
	float CurrentDurationTime = 0.0f;

	// How far along the cooldown we are
	UPROPERTY(BlueprintReadOnly, Category="Ability|State")
	float CurrentCooldownTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Ability|State")
	float AdjustedCooldownTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Ability|State")
	bool bShouldTick = false;

	// True when the ability is done.
	UPROPERTY(BlueprintReadOnly, Category="Ability|State")
	UAbilityComponent* MyAbilityComponent = nullptr;
	

	// Called to activate the ability logic. Override in BP/C++.
	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void Activate(UObject* Context);
	virtual void Activate_Implementation(UObject* Context)
	{
		bIsActive = true;
		ContextObject = Context;
	}

	// Called to deactivate/cleanup.
	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void Deactivate();
	virtual void Deactivate_Implementation()
	{
		bIsActive = false;
		ContextObject = nullptr;
	}

	// Called to actually use the ability.
	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void Init(UAbilityComponent* _MyAbilityComponent);
	virtual void Init_Implementation(UAbilityComponent* _MyAbilityComponent)
	{
		CurrentCooldownTime = AbilityCooldown;
		MyAbilityComponent = _MyAbilityComponent;
	}

	// Called to actually use the ability.
	UFUNCTION(Category="Ability")
	virtual bool StartUsing()
	{
		if (!CanBeUsed()) return false;
		
		StartUsing_Event();
		return true;
	}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ability")
	void StartUsing_Event();
	virtual void StartUsing_Event_Implementation()
	{
		if (AdjustCooldownBasedOnTickedTime) AdjustedCooldownTime = 0.0f;
		CurrentDurationTime = 0.0f;
		bShouldTick = true;
	}

	// Called to tick the use of the ability and check for stop.
	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void TickUse(float DeltaTime);
	virtual void TickUse_Implementation(float DeltaTime)
	{
		if (AdjustCooldownBasedOnTickedTime) AdjustedCooldownTime += DeltaTime;
		
		CurrentDurationTime += DeltaTime;
		
		// GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red,
		// 	FString::Printf(TEXT("CurrentDurationTime: %.3f"), CurrentDurationTime));
	}

	// Called to tick the cooldown of the ability.
	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void TickCooldown(float DeltaTime);
	virtual void TickCooldown_Implementation(float DeltaTime)
	{
		CurrentCooldownTime += DeltaTime;
		// GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red,
		// 	FString::Printf(TEXT("CurrentCooldownTime: %.3f"), CurrentCooldownTime));
	}
	
	// Called when the ability is done.
	UFUNCTION(Category="Ability")
	virtual void StopUsing()
	{
		StopUsing_Event();
	}

	UFUNCTION(BlueprintNativeEvent, Category="Ability")
	void StopUsing_Event();
	virtual void StopUsing_Event_Implementation()
	{
		CurrentCooldownTime = 0.0f;
		bShouldTick = false;
	}
	
	// Getters
	UFUNCTION(BlueprintPure, Category="Ability|Display")
	FText GetAbilityDisplayName() const
	{
		if (!AbilityName.IsEmpty()) return AbilityName;
		return FText::FromString(GetClass()->GetName().Replace(TEXT("BP_"), TEXT("")));
	}

	UFUNCTION(BlueprintPure, Category="Ability|Display")
	FText GetAbilityDisplayDescription() const { return AbilityDescription; }

	UFUNCTION(BlueprintPure, Category="Ability|State")
	bool IsFinished() const
	{
		if (AbilityDuration < 0.0f) return false;
		return CurrentDurationTime > AbilityDuration;
	}

	UFUNCTION(BlueprintPure, Category="Ability|State")
	bool ShouldTick() const { return bShouldTick; }
	
	UFUNCTION(BlueprintPure, Category="Ability|State")
	bool CanBeUsed()
	{
		if (!AdjustCooldownBasedOnTickedTime) AdjustedCooldownTime = AbilityCooldown;

		bool bCanBeUsed = CurrentCooldownTime >= AbilityCooldown || CurrentCooldownTime >= AdjustedCooldownTime+AdditionalCooldownOnAdjustedCooldown;
			
		return bCanBeUsed;
	}
};

