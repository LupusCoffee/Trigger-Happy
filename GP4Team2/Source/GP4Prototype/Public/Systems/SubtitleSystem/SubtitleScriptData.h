// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SubtitleScriptData.generated.h"

class USoundBase;
class USubtitleScriptData;

UENUM(BlueprintType)
enum class Speaker : uint8
{
	None                        UMETA(DisplayName="None"),
	Custom                      UMETA(DisplayName="Custom"),
	Happy                       UMETA(DisplayName="Happy"),
	MissionControl              UMETA(DisplayName="Mission Control"),
	Enemy                       UMETA(DisplayName="Enemy"),
};

UENUM(BlueprintType)
enum class EventType : uint8
{
	None                        UMETA(DisplayName="None"),
	Custom                      UMETA(DisplayName="Custom"),
	MissionControlCallout       UMETA(DisplayName="Mission Control Callout"),
	EnemyShout                  UMETA(DisplayName="Enemy Shout"),
	PlayerVO                    UMETA(DisplayName="Player VO"),
	FloorCleared 				UMETA(DisplayName="Floor Cleared"),
	FewLeft						UMETA(DisplayName="Few Enemies Left"),
};

USTRUCT(BlueprintType)
struct FSubtitleNode
{
	GENERATED_BODY()

	// Event category selected from enum; when Custom, EventCustom is used.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EventType Event = EventType::None;

	// When Event == Custom, this FName is used for matching.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventCustom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* SpecificSound = nullptr;

	// Speaker identity; when Custom, SpeakerCustomText is used for display.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	Speaker SpeakerType = Speaker::None;

	// When Speaker == Custom, this text is used for display.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SpeakerCustomText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine))
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", UIMin="0.0", ToolTip="If > 0, this duration (in seconds) will be used instead of the sound's length"))
	float DurationOverride = 0.f;
	
	bool MatchesEvent(EventType InEventType, FName InCustomEvent, const USubtitleScriptData* Owner) const;
	
	FText ResolveSpeakerText(const USubtitleScriptData* Owner) const;
	
	FName GetSortableEventName(const USubtitleScriptData* Owner) const;
	FName GetSortableSpeakerName(const USubtitleScriptData* Owner) const;
};

UCLASS()
class GP4PROTOTYPE_API USubtitleScriptData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Optional global override for all nodes' Event; when Custom, use EventOverrideCustom.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EventType EventOverride = EventType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventOverrideCustom;

	// Optional global override for all nodes' Speaker; when Custom, use SpeakerOverrideCustomText.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	Speaker SpeakerOverride = Speaker::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SpeakerOverrideCustomText;

	// Optional global duration override (seconds). If > 0, used for all nodes instead of per-node or sound length.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", UIMin="0.0", ToolTip="If > 0, this duration (in seconds) will be used for all entries instead of per-node or sound length"))
	float GlobalDurationOverride = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSubtitleNode> Nodes;
};

// Inline implementations
inline bool FSubtitleNode::MatchesEvent(EventType InEventType, FName InCustomEvent, const USubtitleScriptData* Owner) const
{
	// Apply owner's override first
	EventType EffectiveType = Event;
	FName EffectiveCustom = EventCustom;
	
	if (Owner && Owner->EventOverride != EventType::None)
	{
		EffectiveType = Owner->EventOverride;
		EffectiveCustom = Owner->EventOverrideCustom;
	}

	// Match based on type
	if (EffectiveType == EventType::Custom)
	{
		return InEventType == EventType::Custom && EffectiveCustom == InCustomEvent;
	}
	
	return EffectiveType == InEventType;
}

inline FText FSubtitleNode::ResolveSpeakerText(const USubtitleScriptData* Owner) const
{
	// Apply override first
	Speaker EffectiveSpeaker = SpeakerType;
	FText EffectiveCustomText = SpeakerCustomText;
	
	if (Owner && Owner->SpeakerOverride != Speaker::None)
	{
		EffectiveSpeaker = Owner->SpeakerOverride;
		EffectiveCustomText = Owner->SpeakerOverrideCustomText;
	}

	// Return custom text if speaker is Custom
	if (EffectiveSpeaker == Speaker::Custom)
	{
		return EffectiveCustomText;
	}

	// Return the display name from the enum
	if (UEnum* SpeakerEnum = StaticEnum<Speaker>())
	{
		return SpeakerEnum->GetDisplayNameTextByValue(static_cast<int64>(EffectiveSpeaker));
	}

	return FText::GetEmpty();
}

inline FName FSubtitleNode::GetSortableEventName(const USubtitleScriptData* Owner) const
{
	// Apply owner's override first
	EventType EffectiveType = Event;
	FName EffectiveCustom = EventCustom;
	
	if (Owner && Owner->EventOverride != EventType::None)
	{
		EffectiveType = Owner->EventOverride;
		EffectiveCustom = Owner->EventOverrideCustom;
	}

	// For sorting, return a consistent name based on the event type
	if (EffectiveType == EventType::Custom)
	{
		return FName(*FString::Printf(TEXT("Custom_%s"), *EffectiveCustom.ToString()));
	}
	
	// Use enum value as sortable name
	if (UEnum* EventEnum = StaticEnum<EventType>())
	{
		return FName(*EventEnum->GetNameStringByValue(static_cast<int64>(EffectiveType)));
	}
	
	return NAME_None;
}

inline FName FSubtitleNode::GetSortableSpeakerName(const USubtitleScriptData* Owner) const
{
	// Apply owner's override first
	Speaker EffectiveSpeaker = SpeakerType;
	FText EffectiveCustomText = SpeakerCustomText;
	
	if (Owner && Owner->SpeakerOverride != Speaker::None)
	{
		EffectiveSpeaker = Owner->SpeakerOverride;
		EffectiveCustomText = Owner->SpeakerOverrideCustomText;
	}

	// For sorting, return a consistent name based on the speaker
	if (EffectiveSpeaker == Speaker::Custom)
	{
		return FName(*FString::Printf(TEXT("Custom_%s"), *EffectiveCustomText.ToString()));
	}
	
	// Use enum value as sortable name
	if (UEnum* SpeakerEnum = StaticEnum<Speaker>())
	{
		return FName(*SpeakerEnum->GetNameStringByValue(static_cast<int64>(EffectiveSpeaker)));
	}
	
	return NAME_None;
}
