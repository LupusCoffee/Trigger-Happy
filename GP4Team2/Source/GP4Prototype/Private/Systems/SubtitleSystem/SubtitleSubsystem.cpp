#include "Systems/SubtitleSystem/SubtitleSubsystem.h"

#include "Debug.h"
#include "Blueprint/UserWidget.h"
#include "Components/PanelWidget.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "Components/VerticalBox.h"

#include "Systems/SubtitleSystem/SubtitleScriptData.h"
#include "UI/Widgets/SubtitleWidget.h"
#include "UI/Widgets/SubtitleEntryWidget.h"

void USubtitleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("SubtitleSubsystem: Initialize called"));
}

void USubtitleSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

USubtitleSubsystem* USubtitleSubsystem::GetSubtitleSubsystem(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("SubtitleSubsystem: WorldContextObject is null"));
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("SubtitleSubsystem: World is null"));
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("SubtitleSubsystem: GameInstance is null"));
		return nullptr;
	}

	USubtitleSubsystem* Subsystem = GameInstance->GetSubsystem<USubtitleSubsystem>();
	if (Subsystem)
	{
		UE_LOG(LogTemp, Verbose, TEXT("SubtitleSubsystem: Successfully found subsystem"));
		return Subsystem;
	}

	UE_LOG(LogTemp, Verbose, TEXT("SubtitleSubsystem: Not yet created (WorldType=%d). It will be available once the GameInstance finishes initializing."), static_cast<int32>(World->WorldType));
	return nullptr;
}

void USubtitleSubsystem::InitializeSubtitles(const TArray<USubtitleScriptData*>& InDataAssets, TSubclassOf<UUserWidget> InSubtitleWidgetClass)
{
	DataAssets.Reset();
	for (USubtitleScriptData* DA : InDataAssets)
	{
		if (IsValid(DA))
		{
			DataAssets.Add(DA);
		}
	}
	SubtitleWidgetClass = InSubtitleWidgetClass;
	EnsureWidgetCreated();
}

void USubtitleSubsystem::EnsureWidgetCreated()
{
	if (SubtitleWidgetClass)
	{
		if (!SubtitleWidget)
		{
			if (UWorld* World = GetWorld())
			{
				SubtitleWidget = CreateWidget<USubtitleWidget>(World, SubtitleWidgetClass);
			}
		}
		if (SubtitleWidget && !SubtitleWidget->IsInViewport())
		{
			SubtitleWidget->AddToViewport();
		}
	}
}

void USubtitleSubsystem::HandleSubtitleEvent(EventType Event, FName CustomEvent, USoundBase* PlayedSound)
{
	USubtitleScriptData* OwnerForMatch = nullptr;
	const FSubtitleNode* Node = FindBestNode(Event, CustomEvent, PlayedSound, OwnerForMatch);
	if (!Node)
	{
		UE_LOG(LogTemp, Verbose, TEXT("SubtitleSubsystem: No subtitle node matched (Event=%d, Custom=%s, Sound=%s)"),
			static_cast<int32>(Event), *CustomEvent.ToString(), PlayedSound ? *PlayedSound->GetName() : TEXT("None"));
		return;
	}

	EnsureWidgetCreated();
	if (!SubtitleWidget) return;

	USubtitleEntryWidget* Entry = CreateAndAddEntry(*Node, OwnerForMatch);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("SubtitleSubsystem: Failed to create subtitle entry widget"));
		return;
	}

	const float Duration = ComputeDisplayDuration(*Node, OwnerForMatch, PlayedSound);
	UE_LOG(LogTemp, Log, TEXT("SubtitleSubsystem: Displaying subtitle for %.2f seconds: %s"), Duration, *Node->Line.ToString());
	StartExpiryTimer(Entry, Duration);
}

void USubtitleSubsystem::ClearAllSubtitles()
{
	// Cancel all active timers for tracked subtitles
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		for (FActiveSubtitle& Active : ActiveSubtitles)
		{
			if (TimerManager.IsTimerActive(Active.TimerHandle))
			{
				TimerManager.ClearTimer(Active.TimerHandle);
			}
		}
	}
	ActiveSubtitles.Empty();

	// Clear all subtitle entries from the on-screen widget immediately
	if (SubtitleWidget)
	{
		SubtitleWidget->ClearAllSubtitles();
	}
}

const FSubtitleNode* USubtitleSubsystem::FindNodeBySound(USoundBase* ActualSoundPlayed, USubtitleScriptData*& OutOwner) const
{
	OutOwner = nullptr;
	if (!ActualSoundPlayed || DataAssets.Num() == 0) return nullptr;
	for (USubtitleScriptData* DA : DataAssets)
	{
		if (!IsValid(DA)) continue;
		for (const FSubtitleNode& Node : DA->Nodes)
		{
			if (Node.SpecificSound == ActualSoundPlayed)
			{
				OutOwner = DA;
				return &Node;
			}
		}
	}
	return nullptr;
}

const FSubtitleNode* USubtitleSubsystem::FindMatchingNode(EventType Event, FName CustomEvent, USoundBase* PlayedSound, USubtitleScriptData*& OutOwner) const
{
	OutOwner = nullptr;
	if (DataAssets.Num() == 0) return nullptr;

	// Collect candidates that match the event and have no specific sound bound (event-only lines)
	TArray<TPair<const FSubtitleNode*, USubtitleScriptData*>> Candidates;
	for (USubtitleScriptData* DA : DataAssets)
	{
		if (!IsValid(DA)) continue;
		for (const FSubtitleNode& Node : DA->Nodes)
		{
			if (Node.SpecificSound == nullptr && Node.MatchesEvent(Event, CustomEvent, DA))
			{
				Candidates.Emplace(&Node, DA);
			}
		}
	}
	if (Candidates.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
		OutOwner = Candidates[Index].Value;
		return Candidates[Index].Key;
	}

	return nullptr;
}

// === New helpers ===
const FSubtitleNode* USubtitleSubsystem::FindBestNode(EventType Event, FName CustomEvent, USoundBase* PlayedSound, USubtitleScriptData*& OutOwner) const
{
	OutOwner = nullptr;
	const FSubtitleNode* Node = nullptr;
	if (PlayedSound)
	{
		Node = FindNodeBySound(PlayedSound, OutOwner);
	}
	if (!Node)
	{
		Node = FindMatchingNode(Event, CustomEvent, PlayedSound, OutOwner);
	}
	return Node;
}

float USubtitleSubsystem::ComputeDisplayDuration(const FSubtitleNode& Node, const USubtitleScriptData* OwnerForMatch, USoundBase* PlayedSound) const
{
	// 1) Global override
	if (OwnerForMatch && OwnerForMatch->GlobalDurationOverride > 0.f)
	{
		//Debug::Log(FString::Printf(TEXT("SubtitleSubsystem: Using global duration override: %.2f"), OwnerForMatch->GlobalDurationOverride), true, 5.f);
		return OwnerForMatch->GlobalDurationOverride;
	}
	// 2) Node override
	if (Node.DurationOverride > 0.f)
	{
		//Debug::Log(FString::Printf(TEXT("SubtitleSubsystem: Using node duration override: %.2f"), Node.DurationOverride), true, 5.f);
		return Node.DurationOverride;
	}

	// 3) Sound duration (if reliable)
	if (PlayedSound)
	{
		// Direct Wave
		if (const USoundWave* Wave = Cast<USoundWave>(PlayedSound))
		{
			const float D = Wave->GetDuration();
			if (D > KINDA_SMALL_NUMBER && D < 600.f)
			{
				//Debug::Log(FString::Printf(TEXT("SubtitleSubsystem: Using wave duration: %.2f"), D), true, 5.f);
				return D;
			}
		}

		// Generic sound base
		const float SoundDuration = PlayedSound->GetDuration();
		if (SoundDuration > KINDA_SMALL_NUMBER && SoundDuration < 600.f)
		{
			//Debug::Log(FString::Printf(TEXT("SubtitleSubsystem: Using sound duration: %.2f"), SoundDuration), true, 5.f);
			return SoundDuration;
		}
		//Debug::Log(FString::Printf(TEXT("SubtitleSubsystem: Ignoring invalid/indefinite sound duration: %.2f"), SoundDuration), true, 5.f);
	}

	// 4) Fallback to estimated reading time based on text length
	const float Estimate = EstimateReadingTime(Node.Line);
	//Debug::Log(FString::Printf(TEXT("SubtitleSubsystem: Using estimated reading time: %.2f"), Estimate), true, 5.f);
	return Estimate;
}

float USubtitleSubsystem::EstimateReadingTime(const FText& Text) const
{
	const FString S = Text.ToString();
	if (S.IsEmpty())
	{
		return FallbackMinSeconds;
	}
	int32 LineBreaks = 0;
	for (const TCHAR C : S)
	{
		if (C == '\n') { ++LineBreaks; }
	}
	const int32 CharCount = S.Len();
	const float Base = CharCount / FMath::Max(1.f, FallbackCharsPerSecond);
	const float Extra = LineBreaks * ExtraPerLineBreak;
	return FMath::Clamp(Base + Extra, FallbackMinSeconds, FallbackMaxSeconds);
}

USubtitleEntryWidget* USubtitleSubsystem::CreateAndAddEntry(const FSubtitleNode& Node, const USubtitleScriptData* OwnerForMatch)
{
	if (!SubtitleWidget)
	{
		return nullptr;
	}
	const FText SpeakerText = Node.ResolveSpeakerText(OwnerForMatch);
	USubtitleEntryWidget* Entry = SubtitleWidget->AddSubtitle(SpeakerText, Node.Line);
	if (Entry)
	{
		SubtitleWidget->CullOldEntries();
	}
	return Entry;
}

void USubtitleSubsystem::StartExpiryTimer(USubtitleEntryWidget* Entry, float Duration)
{
	if (!Entry) return;
	if (UWorld* World = GetWorld())
	{
		const int32 TimerId = NextTimerId++;
		FActiveSubtitle Active;
		Active.Entry = Entry;
		Active.StartTime = World->GetTimeSeconds();
		Active.Lifetime = Duration;
		Active.TimerId = TimerId;
		ActiveSubtitles.Add(MoveTemp(Active));

		// Bind and start the timer, then store the handle in the last ActiveSubtitles element
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &USubtitleSubsystem::OnSubtitleExpiredById, TimerId);
		FActiveSubtitle& Last = ActiveSubtitles.Last();
		World->GetTimerManager().SetTimer(Last.TimerHandle, Delegate, Duration, false);
	}
}

void USubtitleSubsystem::OnSubtitleExpiredById(int32 TimerId)
{
	// Find the active subtitle with this timer id
	int32 Index = INDEX_NONE;
	for (int32 i = 0; i < ActiveSubtitles.Num(); ++i)
	{
		if (ActiveSubtitles[i].TimerId == TimerId)
		{
			Index = i;
			break;
		}
	}

	if (Index == INDEX_NONE)
	{
		return;
	}

	USubtitleEntryWidget* Entry = ActiveSubtitles[Index].Entry;

	if (Entry)
	{
		Entry->PlayFadeOutAndDestroy();
	}

	ActiveSubtitles.RemoveAtSwap(Index);
}

void USubtitleSubsystem::SyncWithWidgetChildren()
{
	if (!SubtitleWidget || !SubtitleWidget->SubtitleBox)
	{
		// Without the widget, cancel all timers and clear tracking
		ClearAllSubtitles();
		return;
	}

	UWorld* World = GetWorld();
	FTimerManager* TimerManager = World ? &World->GetTimerManager() : nullptr;

	for (int32 i = ActiveSubtitles.Num() - 1; i >= 0; --i)
	{
		FActiveSubtitle& Active = ActiveSubtitles[i];
		const bool bStillInBox = (SubtitleWidget->SubtitleBox && SubtitleWidget->SubtitleBox->HasChild(Active.Entry));
		if (!Active.Entry || !bStillInBox)
		{
			// Entry was removed (culled or otherwise) -> cancel timer and drop tracking
			if (TimerManager && TimerManager->IsTimerActive(Active.TimerHandle))
			{
				TimerManager->ClearTimer(Active.TimerHandle);
			}
			ActiveSubtitles.RemoveAtSwap(i);
		}
	}
}
