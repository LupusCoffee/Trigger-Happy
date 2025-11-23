#include "Systems/SubtitleSystem/SubtitleBlueprintLibrary.h"

#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Systems/SubtitleSystem/SubtitleSubsystem.h"
#include "Systems/SubtitleSystem/SubtitleScriptData.h"

void USubtitleBlueprintLibrary::InitializeSubtitleSubsystem(UObject* WorldContextObject, const TArray<USubtitleScriptData*>& DataAssets, TSubclassOf<UUserWidget> SubtitleWidgetClass)
{
	if (USubtitleSubsystem* Subsys = USubtitleSubsystem::GetSubtitleSubsystem(WorldContextObject))
	{
		Subsys->InitializeSubtitles(DataAssets, SubtitleWidgetClass);
	}
}

void USubtitleBlueprintLibrary::HandleSubtitleEvent(UObject* WorldContextObject, EventType Event, FName CustomEvent, USoundBase* PlayedSound)
{
	if (USubtitleSubsystem* Subsys = USubtitleSubsystem::GetSubtitleSubsystem(WorldContextObject))
	{
		Subsys->HandleSubtitleEvent(Event, CustomEvent, PlayedSound);
	}
}
