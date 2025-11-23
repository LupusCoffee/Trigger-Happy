#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Systems/SubtitleSystem/SubtitleScriptData.h"
#include "SubtitleBlueprintLibrary.generated.h"

class USubtitleScriptData;
class UUserWidget;
class USoundBase;

UCLASS()
class GP4PROTOTYPE_API USubtitleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Initialize the subtitle subsystem with an array of data assets and optional widget class.
	UFUNCTION(BlueprintCallable, Category="Subtitles", meta=(WorldContext="WorldContextObject"))
	static void InitializeSubtitleSubsystem(UObject* WorldContextObject, const TArray<USubtitleScriptData*>& DataAssets, TSubclassOf<UUserWidget> SubtitleWidgetClass);

	// Single handler: Event + optional CustomEvent + the actual sound that played.
	UFUNCTION(BlueprintCallable, Category="Subtitles", meta=(WorldContext="WorldContextObject", AdvancedDisplay="CustomEvent"))
	static void HandleSubtitleEvent(UObject* WorldContextObject, EventType Event, FName CustomEvent, USoundBase* PlayedSound);
};
