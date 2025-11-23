// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SubtitleEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class GP4PROTOTYPE_API USubtitleEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// BlueprintImplementableEvent → BP defines how to apply style
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetupEntry(const FText& Speaker, const FText& Line);

	// Optional fade-out hook for Blueprint to implement
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Subtitles")
	void PlayFadeOut();

	// Expose animations to C++
	UPROPERTY(BlueprintReadWrite, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOut;

	// Main destroy logic that plays animation first.
	void PlayFadeOutAndDestroy();
};
