// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/SubtitleEntryWidget.h"
#include "Animation/WidgetAnimation.h"
#include "TimerManager.h"

void USubtitleEntryWidget::PlayFadeOutAndDestroy()
{
	// Let Blueprint handle any custom visual logic first
	PlayFadeOut();

	if (FadeOut)
	{
		const float EndTime = FadeOut->GetEndTime();
		PlayAnimation(FadeOut);

		if (UWorld* World = GetWorld())
		{
			if (EndTime > KINDA_SMALL_NUMBER)
			{
				FTimerHandle Handle;
				World->GetTimerManager().SetTimer(
					Handle, [this]()
					{
						if (IsValid(this))
						{
							RemoveFromParent();
						}
					},
					EndTime, false);
			}
		}
	}
	else
	{
		// No animation, remove immediately
		RemoveFromParent();
	}
}
