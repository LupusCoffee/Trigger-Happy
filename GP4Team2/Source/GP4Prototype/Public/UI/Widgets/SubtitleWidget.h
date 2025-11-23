// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SubtitleWidget.generated.h"

class UVerticalBox;
class USubtitleEntryWidget;

/**
 * 
 */
UCLASS()
class GP4PROTOTYPE_API USubtitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Vertical stack in BP
	UPROPERTY(meta=(BindWidget))
	UVerticalBox* SubtitleBox;

	// Entry widget class (set in BP or defaults)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Subtitles")
	TSubclassOf<USubtitleEntryWidget> EntryClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Subtitles")
	float MaxEntryAmount = 3;

	// Adds a subtitle entry to the stack
	UFUNCTION(BlueprintCallable, Category="Subtitles")
	USubtitleEntryWidget* AddSubtitle(const FText& Speaker, const FText& Line);

	// Removes oldest (if you want a max limit)
	void CullOldEntries() const;

	// Clears all active subtitle entries immediately
	UFUNCTION(BlueprintCallable, Category="Subtitles")
	void ClearAllSubtitles();
};
