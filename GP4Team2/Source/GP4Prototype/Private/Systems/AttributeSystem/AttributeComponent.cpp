// Fill out your copyright notice in the Description page of Project Settings.


#include "GP4Prototype/Public/Systems/AttributeSystem/AttributeComponent.h"

#include "Debug.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Systems/UpgradeSystem/UpgradeCardData.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Systems/AttributeSystem/AttributeTags.h"

// -- Lifecycle --
#pragma region Lifecycle

// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	AgentData = nullptr;

	// ...
}

void UAttributeComponent::InitializeComponent()
{
	Super::InitializeComponent();
	EnsureAttributesInitialized();
}

void UAttributeComponent::OnRegister()
{
	Super::OnRegister();
    EnsureAttributesInitialized();
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (AgentData && !bAttributesInitialized)
	{
		InitializeFromAgentData(AgentData);
	}
	
	if (bEnableAttributeDebugLogging && !bAttributesInitialized)
		Debug::LogError(FString::Printf(TEXT("%s: AttributeComponent::BeginPlay: Attributes Not Initialized"),
						   *GetOwner()->GetName()), true, 4.f);
}
#pragma endregion Lifecycle

// -- Blueprint API: Getter/Setter --
#pragma region BlueprintAPI_GetterSetter

FAttribute UAttributeComponent::GetAttribute(FGameplayTag tag)
{
	auto attribute = Attributes.Find(tag);
	if (attribute != nullptr)
	{
		return *attribute;
	}
	return FAttribute();
}

FGameplayTag UAttributeComponent::GetAttributeTag(FAttribute attribute)
{
	for (const auto& Elem : Attributes)
	{
		if (Elem.Value.BaseValue == attribute.BaseValue && Elem.Value.Value == attribute.Value)
		{
			return Elem.Key;
		}
	}
	return FGameplayTag();
}

float UAttributeComponent::GetAttributeBaseValue(FGameplayTag tag) const
{
	auto attribute = Attributes.Find(tag);
	if (attribute != nullptr)
	{
		return attribute->BaseValue;
	}
	return 0.0f;
}

float UAttributeComponent::GetAttributeValue(FGameplayTag tag) const
{
	auto attribute = Attributes.Find(tag);
	if (attribute != nullptr)
	{
		return attribute->Value;
	}
	return 0.0f;
}

int32 UAttributeComponent::GetAttributeValueInt(FGameplayTag Tag) const
{
	return FMath::RoundToInt(GetAttributeValue(Tag));
}

float UAttributeComponent::GetAttributeClamped(FGameplayTag Tag, float Min, float Max) const
{
	return FMath::Clamp(GetAttributeValue(Tag), Min, Max);
}

bool UAttributeComponent::HasAttribute(FGameplayTag Tag) const
{
	return Attributes.Contains(Tag);
}

bool UAttributeComponent::HasModifier(FGameplayTag Tag, FGuid ID) const
{
	if (const FAttribute* Attr = Attributes.Find(Tag))
	{
		return Attr->ActiveModifiers.ContainsByPredicate([&](const FModifier& M){ return M.ModifierID == ID; });
	}
	return false;
}

int32 UAttributeComponent::GetAppliedModifierRefCountForSource(UObject* Source) const
{
	if (!Source)
	{
		return 0;
	}
	if (const TArray<FAppliedModifierRef>* RefsPtr = SourceAppliedModifiers.Find(Source))
	{
		return RefsPtr->Num();
	}
	return 0;
}

void UAttributeComponent::SetAttributeBaseValue(FGameplayTag tag, float NewValue)
{
	if (FAttribute* Attribute = Attributes.Find(tag))
	{
		if (Attribute->NumericType == EAttributeNumericType::Integer)
		{
			NewValue = static_cast<float>(FMath::RoundToInt(NewValue));
		}
		const float OldBase = Attribute->BaseValue;
		if (!FMath::IsNearlyEqual(OldBase, NewValue))
		{
			Attribute->BaseValue = NewValue;
			const float OldValue = Attribute->Value;
			Attribute->Recalculate();
			const float NewEffective = Attribute->Value;

			OnAttributeBaseChanged.Broadcast(tag, OldBase, NewValue);

			if (!FMath::IsNearlyEqual(OldValue, NewEffective))
			{
				OnAttributeValueChanged.Broadcast(tag, OldValue, NewEffective);
				OnAnyAttributeChanged.Broadcast(tag, OldValue, NewEffective);
			}
		}
	}
}

void UAttributeComponent::SetAttributeValue(FGameplayTag tag, float NewValue)
{
	if (FAttribute* Attribute = Attributes.Find(tag))
	{
		if (Attribute->NumericType == EAttributeNumericType::Integer)
		{
			NewValue = static_cast<float>(FMath::RoundToInt(NewValue));
		}
		// Enforce attribute-level clamp when setting directly
		switch (Attribute->ClampMode)
		{
		case EAttributeClampMode::Max:
			NewValue = FMath::Min(NewValue, Attribute->ClampValue);
			break;
		case EAttributeClampMode::Min:
			NewValue = FMath::Max(NewValue, Attribute->ClampValue);
			break;
		case EAttributeClampMode::None:
		default:
			break;
		}
		const float OldValue = Attribute->Value;
		if (!FMath::IsNearlyEqual(OldValue, NewValue))
		{
			Attribute->Value = NewValue;
			OnAttributeValueChanged.Broadcast(tag, OldValue, NewValue);
			OnAnyAttributeChanged.Broadcast(tag, OldValue, NewValue);
		}
	}
}

void UAttributeComponent::ResetAttribute(FGameplayTag Tag)
{
	if (FAttribute* Attr = Attributes.Find(Tag))
	{
		if (Attr->ActiveModifiers.Num() > 0)
		{
			ClearModifiers(Tag);
		}
		else
		{
			const float OldValue = Attr->Value;
			Attr->Recalculate();
			const float NewValue = Attr->Value;
			if (!FMath::IsNearlyEqual(OldValue, NewValue))
			{
				OnAttributeValueChanged.Broadcast(Tag, OldValue, NewValue);
				OnAnyAttributeChanged.Broadcast(Tag, OldValue, NewValue);
			}
		}
	}
}
#pragma endregion BlueprintAPI_GetterSetter

// -- Blueprint API: Modifier Management --
#pragma region BlueprintAPI_Modifiers

void UAttributeComponent::AddModifier(FGameplayTag Tag, const FModifier& Modifier)
{
	if (FAttribute* Attribute = Attributes.Find(Tag))
	{
		FModifier Copy = Modifier;
		if (!Copy.ModifierID.IsValid())
		{
			Copy.ModifierID = FGuid::NewGuid();
		}

		// If this attribute is Integer-typed, coerce Addition/Override values to int
		if (Attribute->NumericType == EAttributeNumericType::Integer)
		{
			if (Copy.Type == EModificationType::Addition || Copy.Type == EModificationType::Override)
			{
				Copy.Value = static_cast<float>(FMath::RoundToInt(Copy.Value));
			}
		}
		
		Attribute->ActiveModifiers.Add(Copy);

		OnAttributeModified.Broadcast(Tag, Copy);

		// Recalculate and broadcast change if any
		const float OldValue = Attribute->Value;
		Attribute->Recalculate();
		const float NewValue = Attribute->Value;
		if (!FMath::IsNearlyEqual(OldValue, NewValue))
		{
			OnAttributeValueChanged.Broadcast(Tag, OldValue, NewValue);
			OnAnyAttributeChanged.Broadcast(Tag, OldValue, NewValue);
		}
	}
}

// Convenience: add and register from a source
FGuid UAttributeComponent::AddModifierFromSource(FGameplayTag Tag, const FModifier& Modifier, UObject* Source)
{
	FModifier Copy = Modifier;
	if (!Copy.ModifierID.IsValid())
	{
		Copy.ModifierID = FGuid::NewGuid();
	}
	AddModifier(Tag, Copy);
	if (Source)
	{
		RegisterAppliedModifier(Source, Tag, Copy.ModifierID);
	}
	return Copy.ModifierID;
}

FGuid UAttributeComponent::AddTemporaryModifier(FGameplayTag Tag, const FModifier& Modifier, float DurationSeconds, UObject* Source)
{
	const FGuid NewID = AddModifierFromSource(Tag, Modifier, Source);

	if (DurationSeconds > 0.f && GetWorld())
	{
		FTimerDelegate D;
		D.BindUObject(this, &UAttributeComponent::HandleTempModifierExpired, NewID);
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, D, DurationSeconds, false);
		TempRemovalTimers.Add(NewID, Handle);
		TempIdToTag.Add(NewID, Tag);
	}
	return NewID;
}

void UAttributeComponent::AddModifiers(FGameplayTag Tag, const TArray<FModifier>& Modifiers)
{
	for (const FModifier& M : Modifiers)
	{
		AddModifier(Tag, M);
	}
}


void UAttributeComponent::RemoveModifier(FGameplayTag Tag, const FModifier& Modifier)
{
	if (FAttribute* Attribute = Attributes.Find(Tag))
	{
		int32 Removed = Attribute->ActiveModifiers.RemoveAll([&](const FModifier& M)
		{
			// Prefer exact ID match when available; otherwise fall back to a conservative field comparison
			if (Modifier.ModifierID.IsValid())
			{
				return M.ModifierID == Modifier.ModifierID;
			}
			return (M.Type == Modifier.Type)
				&& FMath::IsNearlyEqual(M.Value, Modifier.Value);
		});

		if (Removed > 0)
		{
			OnAttributeModified.Broadcast(Tag, Modifier);

			const float OldValue = Attribute->Value;
			Attribute->Recalculate();
			const float NewValue = Attribute->Value;
			if (!FMath::IsNearlyEqual(OldValue, NewValue))
			{
				OnAttributeValueChanged.Broadcast(Tag, OldValue, NewValue);
				OnAnyAttributeChanged.Broadcast(Tag, OldValue, NewValue);
			}
		}
	}
}

void UAttributeComponent::ClearModifiers(FGameplayTag Tag)
{
	if (FAttribute* Attribute = Attributes.Find(Tag))
	{
		if (Attribute->ActiveModifiers.Num() == 0)
		{
			return;
		}

		// Broadcast each removal for traceability
		for (const FModifier& M : Attribute->ActiveModifiers)
		{
			OnAttributeModified.Broadcast(Tag, M);
		}

		const float OldValue = Attribute->Value;
		Attribute->ActiveModifiers.Empty();
		Attribute->Recalculate();
		const float NewValue = Attribute->Value;
		if (!FMath::IsNearlyEqual(OldValue, NewValue))
		{
			OnAttributeValueChanged.Broadcast(Tag, OldValue, NewValue);
			OnAnyAttributeChanged.Broadcast(Tag, OldValue, NewValue);
		}
	}
}

void UAttributeComponent::RegisterAppliedModifier(UObject* Source, FGameplayTag Tag, FGuid ModifierID)
{
	if (!Source || !ModifierID.IsValid())
	{
		return;
	}

	FAppliedModifierRef Ref;
	Ref.Tag = Tag;
	Ref.ModifierID = ModifierID;
	SourceAppliedModifiers.FindOrAdd(Source).Add(Ref);
}

void UAttributeComponent::UnregisterAppliedModifier(UObject* Source, FGuid ModifierID)
{
	if (!Source || !ModifierID.IsValid())
	{
		return;
	}
	if (TArray<FAppliedModifierRef>* RefsPtr = SourceAppliedModifiers.Find(Source))
	{
		RefsPtr->RemoveAll([&](const FAppliedModifierRef& R){ return R.ModifierID == ModifierID; });
		if (RefsPtr->Num() == 0)
		{
			SourceAppliedModifiers.Remove(Source);
		}
	}
}

void UAttributeComponent::RemoveAllModifiersFromSourceObject(UObject* Source)
{
	if (!Source)
	{
		return;
	}

	// Look up refs for this source
	TArray<FAppliedModifierRef>* RefsPtr = SourceAppliedModifiers.Find(Source);
	if (!RefsPtr)
	{
		return;
	}

	// Copy then remove the entry to avoid re-entrancy issues
	TArray<FAppliedModifierRef> Refs = *RefsPtr;
	SourceAppliedModifiers.Remove(Source);

	for (const FAppliedModifierRef& Ref : Refs)
	{
		if (FAttribute* Attr = Attributes.Find(Ref.Tag))
		{
			const float OldValue = Attr->Value;
			bool bAnyRemoved = false;
			Attr->ActiveModifiers.RemoveAll([&](const FModifier& M)
			{
				if (M.ModifierID == Ref.ModifierID)
				{
					bAnyRemoved = true;
					return true;
				}
				return false;
			});

			if (bAnyRemoved)
			{
				// Emit a minimal modified event carrying the ID
				FModifier Temp; Temp.ModifierID = Ref.ModifierID;
				OnAttributeModified.Broadcast(Ref.Tag, Temp);

				Attr->Recalculate();
				const float NewValue = Attr->Value;
				if (!FMath::IsNearlyEqual(OldValue, NewValue))
				{
					OnAttributeValueChanged.Broadcast(Ref.Tag, OldValue, NewValue);
					OnAnyAttributeChanged.Broadcast(Ref.Tag, OldValue, NewValue);
				}
			}
		}
	}
}

// Convenience: remove by ID, auto-resolving tag if needed
bool UAttributeComponent::RemoveModifierByID(FGameplayTag Tag, FGuid ModifierID)
{
	if (!ModifierID.IsValid()) return false;

	FGameplayTag TagToUse = Tag;
	if (!TagToUse.IsValid())
	{
		if (const FGameplayTag* FoundTag = TempIdToTag.Find(ModifierID))
		{
			TagToUse = *FoundTag;
		}
		else
		{
			// Fallback: scan attributes to find which tag contains this modifier
			for (const auto& KVP : Attributes)
			{
				if (KVP.Value.ActiveModifiers.ContainsByPredicate([&](const FModifier& M){ return M.ModifierID == ModifierID; }))
				{
					TagToUse = KVP.Key;
					break;
				}
			}
		}
	}

	if (!TagToUse.IsValid()) return false;

	if (FAttribute* Attribute = Attributes.Find(TagToUse))
	{
		const float OldValue = Attribute->Value;
		int32 Removed = Attribute->ActiveModifiers.RemoveAll([&](const FModifier& M){ return M.ModifierID == ModifierID; });
		if (Removed > 0)
		{
			// Emit minimal modified event
			FModifier Temp; Temp.ModifierID = ModifierID;
			OnAttributeModified.Broadcast(TagToUse, Temp);

			Attribute->Recalculate();
			const float NewValue = Attribute->Value;
			if (!FMath::IsNearlyEqual(OldValue, NewValue))
			{
				OnAttributeValueChanged.Broadcast(TagToUse, OldValue, NewValue);
				OnAnyAttributeChanged.Broadcast(TagToUse, OldValue, NewValue);
			}
			return true;
		}
	}
	return false;
}

bool UAttributeComponent::CancelTemporaryModifier(FGuid ModifierID)
{
	if (!ModifierID.IsValid()) return false;

	// Clear timer if present
	if (FTimerHandle* HandlePtr = TempRemovalTimers.Find(ModifierID))
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(*HandlePtr);
		}
		TempRemovalTimers.Remove(ModifierID);
	}

	// Remove modifier and clean maps
	const FGameplayTag* TagPtr = TempIdToTag.Find(ModifierID);
	bool bRemoved = false;
	if (TagPtr)
	{
		bRemoved = RemoveModifierByID(*TagPtr, ModifierID);
		TempIdToTag.Remove(ModifierID);
	}
	else
	{
		bRemoved = RemoveModifierByID(FGameplayTag{}, ModifierID);
	}

	// Remove the source ref wherever it exists
	for (auto It = SourceAppliedModifiers.CreateIterator(); It; ++It)
	{
		TArray<FAppliedModifierRef>& Arr = It.Value();
		Arr.RemoveAll([&](const FAppliedModifierRef& R){ return R.ModifierID == ModifierID; });
		if (Arr.Num() == 0)
		{
			It.RemoveCurrent();
		}
	}
	return bRemoved;
}

void UAttributeComponent::CancelAllTemporaryModifiersFromSource(UObject* Source)
{
	if (!Source) return;
	TArray<FAppliedModifierRef>* RefsPtr = SourceAppliedModifiers.Find(Source);
	if (!RefsPtr) return;

	// Copy to avoid mutation issues
	TArray<FAppliedModifierRef> Refs = *RefsPtr;
	for (const FAppliedModifierRef& Ref : Refs)
	{
		if (TempIdToTag.Contains(Ref.ModifierID))
		{
			CancelTemporaryModifier(Ref.ModifierID);
		}
	}
}

void UAttributeComponent::HandleTempModifierExpired(FGuid ModifierID)
{
	// Timer fired: remove modifier, cleanup tracking, and unregister source refs
	const FGameplayTag* TagPtr = TempIdToTag.Find(ModifierID);
	if (TagPtr)
	{
		RemoveModifierByID(*TagPtr, ModifierID);
		TempIdToTag.Remove(ModifierID);
	}
	TempRemovalTimers.Remove(ModifierID);

	for (auto It = SourceAppliedModifiers.CreateIterator(); It; ++It)
	{
		TArray<FAppliedModifierRef>& Arr = It.Value();
		Arr.RemoveAll([&](const FAppliedModifierRef& R){ return R.ModifierID == ModifierID; });
		if (Arr.Num() == 0)
		{
			It.RemoveCurrent();
		}
	}
}

#pragma endregion // BlueprintAPI_Modifiers

// -- Blueprint API: Recalculation --
#pragma region BlueprintAPI_Recalculate

void UAttributeComponent::RecalculateAttribute(FGameplayTag Tag)
{
	if (FAttribute* Attribute = Attributes.Find(Tag))
	{
		const float OldValue = Attribute->Value;
		Attribute->Recalculate();
		const float NewValue = Attribute->Value;
		if (!FMath::IsNearlyEqual(OldValue, NewValue))
		{
			OnAttributeValueChanged.Broadcast(Tag, OldValue, NewValue);
			OnAnyAttributeChanged.Broadcast(Tag, OldValue, NewValue);
		}
	}
}

void UAttributeComponent::RecalculateAll()
{
	for (auto& KVP : Attributes)
	{
		RecalculateAttribute(KVP.Key);
	}
}
#pragma endregion BlueprintAPI_Recalculate

// -- Internal API --
#pragma region InternalAPI

void UAttributeComponent::InitializeFromAgentData(UAgentData* newAgentData)
{
	if (!newAgentData)
	{
		if (bEnableAttributeDebugLogging)
		{
			FString OwnerName = GetOwner()->GetName();
			Debug::LogError(FString::Printf(TEXT("%s: AttributeComponent::InitializeFromAgentData: newAgentData is null!"), *OwnerName), true, 50.f);
		}
		return;
	}
	if (bAttributesInitialized && newAgentData == AgentData) return; 
	AgentData = newAgentData;

	// Do NOT clear the map to preserve references returned by RegisterAndGetAttribute
	for (const FAgentAttribute & Entry : newAgentData->Attributes)
	{
		FAttribute & Attr = Attributes.FindOrAdd(Entry.AttributeTag);

		const float OldBase = Attr.BaseValue;
		const float OldValue = Attr.Value;

		Attr.BaseValue   = Entry.BaseValue;
		Attr.NumericType = Entry.NumericType;
		Attr.RoundingMode = Entry.RoundingMode;
		Attr.ClampMode = Entry.ClampMode;
		Attr.ClampValue = Entry.ClampValue;
		if (Attr.NumericType == EAttributeNumericType::Integer)
		{
			Attr.BaseValue = static_cast<float>(FMath::RoundToInt(Attr.BaseValue));
			if (Attr.ClampMode != EAttributeClampMode::None)
			{
				Attr.ClampValue = static_cast<float>(FMath::RoundToInt(Attr.ClampValue));
			}
		}
		Attr.Recalculate();

		const float NewBase = Attr.BaseValue;
		const float NewValue = Attr.Value;

		if (!FMath::IsNearlyEqual(OldBase, NewBase))
		{
			OnAttributeBaseChanged.Broadcast(Entry.AttributeTag, OldBase, NewBase);
		}
		if (!FMath::IsNearlyEqual(OldValue, NewValue))
		{
			OnAttributeValueChanged.Broadcast(Entry.AttributeTag, OldValue, NewValue);
			OnAnyAttributeChanged.Broadcast(Entry.AttributeTag, OldValue, NewValue);
		}

		// Diagnostic: warn if clamp immediately forces value away from base (common cause of unexpected zeros)
		if (Attr.ClampMode == EAttributeClampMode::Max && Attr.ClampValue < NewBase && FMath::IsNearlyEqual(NewValue, Attr.ClampValue))
		{
			if (bEnableAttributeDebugLogging) Debug::LogWarning(FString::Printf(TEXT("Attribute '%s' was clamped by Max ClampValue %.2f (Base=%.2f). Result=%.2f"), *Entry.AttributeTag.ToString(), Attr.ClampValue, NewBase, NewValue), true, 100.f);
		}
		else if (Attr.ClampMode == EAttributeClampMode::Min && Attr.ClampValue > NewBase && FMath::IsNearlyEqual(NewValue, Attr.ClampValue))
		{
			if (bEnableAttributeDebugLogging) Debug::LogWarning(FString::Printf(TEXT("Attribute '%s' was raised by Min ClampValue %.2f (Base=%.2f). Result=%.2f"), *Entry.AttributeTag.ToString(), Attr.ClampValue, NewBase, NewValue), true, 100.f);
		}
		if (bEnableAttributeDebugLogging)
		{
			Debug::Log(FString::Printf(TEXT("%s Attribute '%s' initialized: Base=%.2f, Value=%.2f"), *GetOwner()->GetName(), *Entry.AttributeTag.ToString(), NewBase, NewValue), true, 2.f);
		}
	}
	
	bAttributesInitialized = true;
	OnAttributeInitialized.Broadcast();
}

FGameplayTag GetTagFromName(FName Name)
{
	return FGameplayTag::RequestGameplayTag(Name);
}

#if WITH_EDITOR
#include "Misc/ConfigCacheIni.h"

void AddTagToConfig(const FString& TagName, const FString& Comment)
{
	FString Section = "/Script/GameplayTags.GameplayTagsList";
	FString Entry = FString::Printf(TEXT("(Tag=\"%s\",DevComment=\"%s\")"), *TagName, *Comment);

	// Add to DefaultGameplayTags.ini if not already there
	if (!GConfig->DoesSectionExist(*Section, GGameIni))
	{
		GConfig->SetArray(*Section, TEXT("GameplayTagList"), {Entry}, GGameIni);
		GConfig->Flush(false, GGameIni);
	}
}
#endif

FAttribute& UAttributeComponent::RegisterAndGetAttribute(const FString& TagName,
                                                         const FString& DevComment /*= TEXT("")*/)
{
	// Tags must be declared ahead of time (native or config). Do not register at runtime.
	FGameplayTag Tag = AttributeTags::FindTagByString(TagName);
	if (!Tag.IsValid())
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		Tag = Manager.RequestGameplayTag(FName(*TagName), /*ErrorIfNotFound=*/true);
#if WITH_EDITOR
		if (!Tag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("RegisterAndGetAttribute: Tag '%s' is not declared natively or in DefaultGameplayTags.ini. Please add it to GP4AttributeTags or config."), *TagName);
		}
#endif
	}

	return Attributes.FindOrAdd(Tag);
}

void UAttributeComponent::EnsureAttributesInitialized()
{
    if (bAttributesInitialized || !AgentData)
    {
    	if (bEnableAttributeDebugLogging)
    		Debug::LogWarning("AttributeComponent::EnsureAttributesInitialized: AgentData is null, cannot initialize!", true, 4.f);
	    return;
    }

	InitializeFromAgentData(AgentData);
}
#pragma endregion InternalAPI

// -- Rounding API --
EAttributeRoundingMode UAttributeComponent::GetAttributeRoundingMode(FGameplayTag Tag) const
{
	if (const FAttribute* Attribute = Attributes.Find(Tag))
	{
		return Attribute->RoundingMode;
	}
	return EAttributeRoundingMode::None;
}

EAttributeNumericType UAttributeComponent::GetAttributeNumericType(FGameplayTag Tag) const
{
	if (const FAttribute* Attribute = Attributes.Find(Tag))
	{
		return Attribute->NumericType;
	}
	return EAttributeNumericType::Float;
}

void UAttributeComponent::SetAttributeNumericType(FGameplayTag Tag, EAttributeNumericType Type)
{
	if (FAttribute* Attribute = Attributes.Find(Tag))
	{
		if (Attribute->NumericType != Type)
		{
			const float OldValue = Attribute->Value;
			Attribute->NumericType = Type;
			Attribute->Recalculate();
			const float NewValue = Attribute->Value;
			if (!FMath::IsNearlyEqual(OldValue, NewValue))
			{
				OnAttributeValueChanged.Broadcast(Tag, OldValue, NewValue);
				OnAnyAttributeChanged.Broadcast(Tag, OldValue, NewValue);
			}
		}
	}
}

void UAttributeComponent::SetAttributeRoundingMode(FGameplayTag Tag, EAttributeRoundingMode Mode)
{
	if (FAttribute* Attribute = Attributes.Find(Tag))
	{
		if (Attribute->RoundingMode != Mode)
		{
			const float OldValue = Attribute->Value;
			Attribute->RoundingMode = Mode;
			Attribute->Recalculate();
			const float NewValue = Attribute->Value;
			if (!FMath::IsNearlyEqual(OldValue, NewValue))
			{
				OnAttributeValueChanged.Broadcast(Tag, OldValue, NewValue);
				OnAnyAttributeChanged.Broadcast(Tag, OldValue, NewValue);
			}
		}
	}
}

// -- Preview helpers --
FAttributePreviewResult UAttributeComponent::PreviewApplyModifierForTag(FGameplayTag Tag, const FModifier& NewModifier) const
{
	TArray<FModifier> Mods; Mods.Add(NewModifier);
	return PreviewApplyModifiersForTag(Tag, Mods);
}

FAttributePreviewResult UAttributeComponent::PreviewApplyModifiersForTag(FGameplayTag Tag, const TArray<FModifier>& NewModifiers) const
{
	FAttributePreviewResult Result; Result.Attribute = Tag;
	const float OldValRaw = GetAttributeValue(Tag);

	const FAttribute* Attr = Attributes.Find(Tag);
	// If attribute does not exist yet, preview against a default attribute (0 base)
	FAttribute TempAttr;
	if (!Attr)
	{
		Attr = &TempAttr;
	}

	// Accumulate current active modifiers + incoming ones
	float Additive = 0.f;
	float Mult = 1.f;
	TOptional<float> OverrideVal;
	// Track whether any additive or multiplicative modifier was introduced by new modifiers (or existing ones)
	bool bAnyAdditive = false;
	bool bAnyMultiplicative = false;
	// Track whether NEW modifiers include an Override (used by UI even if Delta==0)
	bool bOverrideFromNew = false;

	auto Accumulate = [&](const FModifier& M)
	{
		switch (M.Type)
		{
		case EModificationType::Addition:
			Additive += M.Value; bAnyAdditive = true; break;
		case EModificationType::Subtraction:
			Additive -= M.Value; bAnyAdditive = true; break;
		case EModificationType::Multiplication:
			Mult *= M.Value; bAnyMultiplicative = true; break;
		case EModificationType::Override:
			OverrideVal = M.Value; break;
		}
	};

	for (const FModifier& M : Attr->ActiveModifiers)
	{
		Accumulate(M);
	}

	// Include new modifiers (coerce integer semantics for Add/Override like AddModifier does)
	for (FModifier M : NewModifiers)
	{
		if (Attr->NumericType == EAttributeNumericType::Integer && (M.Type == EModificationType::Addition || M.Type == EModificationType::Override))
		{
			M.Value = static_cast<float>(FMath::RoundToInt(M.Value));
		}
		if (M.Type == EModificationType::Override)
		{
			bOverrideFromNew = true;
		}
		Accumulate(M);
	}

	float NewVal = Attr->BaseValue;
	NewVal = (NewVal + Additive) * Mult;
	if (OverrideVal.IsSet())
	{
		NewVal = OverrideVal.GetValue();
	}

	// Capture raw values prior to rounding/clamp for preview transparency
	Result.RawOldValue = OldValRaw;
	Result.RawNewValue = NewVal;

	// Integer rounding policy on NEW value
	if (Attr->NumericType == EAttributeNumericType::Integer)
	{
		switch (Attr->RoundingMode)
		{
		case EAttributeRoundingMode::RoundToNearest:
			NewVal = static_cast<float>(FMath::RoundToInt(NewVal)); break;
		case EAttributeRoundingMode::None:
		default:
			NewVal = static_cast<float>(FMath::RoundToInt(NewVal)); break;
		}
	}

	// Attribute-level clamp
	if (Attr->ClampMode == EAttributeClampMode::Max && NewVal > Attr->ClampValue)
	{
		Result.bClampedByAttributeMax = true; // legacy flag for Max
		Result.ClampApplied = EAttributeClampMode::Max;
		NewVal = Attr->ClampValue;
		if (Attr->NumericType == EAttributeNumericType::Integer)
		{
			NewVal = static_cast<float>(FMath::RoundToInt(NewVal));
		}
	}
	else if (Attr->ClampMode == EAttributeClampMode::Min && NewVal < Attr->ClampValue)
	{
		Result.ClampApplied = EAttributeClampMode::Min;
		NewVal = Attr->ClampValue;
		if (Attr->NumericType == EAttributeNumericType::Integer)
		{
			NewVal = static_cast<float>(FMath::RoundToInt(NewVal));
		}
	}

	// Prepare OLD display value respecting integer semantics
	float OldValDisplay = OldValRaw;
	if (Attr->NumericType == EAttributeNumericType::Integer)
	{
		OldValDisplay = static_cast<float>(FMath::RoundToInt(OldValDisplay));
	}

	Result.OldValue = OldValDisplay;
	Result.NewValue = NewVal;
	Result.Delta = Result.NewValue - Result.OldValue;
	Result.bUnderlyingChangeMaskedByRounding = FMath::IsNearlyZero(Result.Delta) && !FMath::IsNearlyZero(Result.RawNewValue - Result.RawOldValue);
	// Populate new metadata
	Result.bHasAdditiveChange = bAnyAdditive;
	Result.bHasMultiplicativeChange = bAnyMultiplicative;
	Result.bHasOverrideChange = bOverrideFromNew;
	Result.NetAdditiveDelta = Additive;
	Result.NetMultiplicativeFactor = Mult;
	return Result;
}

void UAttributeComponent::DumpAttributes() const
{
	UE_LOG(LogTemp, Log, TEXT("=== AttributeComponent Dump (%s) ==="), *GetOwner()->GetName());
	for (const auto& KVP : Attributes)
	{
		const FGameplayTag& Tag = KVP.Key;
		const FAttribute& Attr = KVP.Value;
		FString NumericTypeStr = Attr.NumericType == EAttributeNumericType::Integer ? TEXT("Integer") : TEXT("Float");
		FString ClampModeStr = TEXT("None");
		switch (Attr.ClampMode)
		{
		case EAttributeClampMode::Min: ClampModeStr = TEXT("Min"); break; 
		case EAttributeClampMode::Max: ClampModeStr = TEXT("Max"); break; 
		default: break;
		}
		UE_LOG(LogTemp, Log, TEXT("Tag=%s Base=%.2f Value=%.2f Type=%s ClampMode=%s ClampValue=%.2f Modifiers=%d"), *Tag.ToString(), Attr.BaseValue, Attr.Value, *NumericTypeStr, *ClampModeStr, Attr.ClampValue, Attr.ActiveModifiers.Num());
		for (const FModifier& M : Attr.ActiveModifiers)
		{
			FString ModType;
			switch (M.Type)
			{
			case EModificationType::Addition: ModType = TEXT("Add"); break; 
			case EModificationType::Subtraction: ModType = TEXT("Sub"); break; 
			case EModificationType::Multiplication: ModType = TEXT("Mult"); break; 
			case EModificationType::Override: ModType = TEXT("Override"); break; 
			}
			UE_LOG(LogTemp, Log, TEXT("    - %s %.3f ID=%s"), *ModType, M.Value, *M.ModifierID.ToString());
		}
	}
}

