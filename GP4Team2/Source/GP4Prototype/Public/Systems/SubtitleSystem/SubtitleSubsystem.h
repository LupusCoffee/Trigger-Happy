#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Systems/SubtitleSystem/SubtitleScriptData.h"
#include "UI/Widgets/SubtitleWidget.h"
#include "TimerManager.h"
#include "SubtitleSubsystem.generated.h"

class USubtitleEntryWidget;

USTRUCT()
struct FActiveSubtitle
{
	GENERATED_BODY()

	UPROPERTY()
	USubtitleEntryWidget* Entry = nullptr;

	UPROPERTY()
	float StartTime = 0.f;

	UPROPERTY()
	float Lifetime = 0.f;

	UPROPERTY()
	int32 TimerId = 0;

	// Handle to cancel/inspect the expiration timer
	UPROPERTY(Transient)
	FTimerHandle TimerHandle;
};

UCLASS(BlueprintType, Blueprintable)
class GP4PROTOTYPE_API USubtitleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	
	UFUNCTION(BlueprintCallable, Category="Subtitles", meta=(WorldContext="WorldContextObject"))
	static USubtitleSubsystem* GetSubtitleSubsystem(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category="Subtitles")
	void InitializeSubtitles(const TArray<USubtitleScriptData*>& InDataAssets, TSubclassOf<UUserWidget> InSubtitleWidgetClass);

	UFUNCTION(BlueprintCallable, Category="Subtitles")
	void HandleSubtitleEvent(EventType Event, FName CustomEvent, USoundBase* PlayedSound);

	UFUNCTION(BlueprintCallable, Category="Subtitles")
	void ClearAllSubtitles();

	const TArray<USubtitleScriptData*>& GetDataAssets() const { return DataAssets; }

protected:
	const FSubtitleNode* FindNodeBySound(USoundBase* ActualSoundPlayed, USubtitleScriptData*& OutOwner) const;
	const FSubtitleNode* FindMatchingNode(EventType Event, FName CustomEvent, USoundBase* PlayedSound, USubtitleScriptData*& OutOwner) const;

	void EnsureWidgetCreated();

	const FSubtitleNode* FindBestNode(EventType Event, FName CustomEvent, USoundBase* PlayedSound, USubtitleScriptData*& OutOwner) const;
	float ComputeDisplayDuration(const FSubtitleNode& Node, const USubtitleScriptData* OwnerForMatch, USoundBase* PlayedSound) const;
	float EstimateReadingTime(const FText& Text) const;
	USubtitleEntryWidget* CreateAndAddEntry(const FSubtitleNode& Node, const USubtitleScriptData* OwnerForMatch);
	void StartExpiryTimer(USubtitleEntryWidget* Entry, float Duration);
	UFUNCTION()
	void OnSubtitleExpiredById(int32 TimerId);
	void SyncWithWidgetChildren();
	
	UPROPERTY(EditDefaultsOnly, Category="Subtitles")
	TArray<USubtitleScriptData*> DataAssets;

	UPROPERTY(EditDefaultsOnly, Category="Subtitles")
	TSubclassOf<UUserWidget> SubtitleWidgetClass;

	UPROPERTY(Transient)
	USubtitleWidget* SubtitleWidget = nullptr;

	// Strongly tracked active subtitles with timing and timer handle
	UPROPERTY(Transient)
	TArray<FActiveSubtitle> ActiveSubtitles;

	UPROPERTY(Transient)
	int32 NextTimerId = 1;

	// Fallback heuristics for duration when sound length is unknown/invalid
	UPROPERTY(EditAnywhere, Category="Subtitles|Timing")
	float FallbackCharsPerSecond = 14.f; // reading speed

	UPROPERTY(EditAnywhere, Category="Subtitles|Timing")
	float FallbackMinSeconds = 1.5f;

	UPROPERTY(EditAnywhere, Category="Subtitles|Timing")
	float FallbackMaxSeconds = 8.0f;

	UPROPERTY(EditAnywhere, Category="Subtitles|Timing")
	float ExtraPerLineBreak = 0.35f;
};
