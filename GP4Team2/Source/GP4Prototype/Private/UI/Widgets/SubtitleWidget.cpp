// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/SubtitleWidget.h"
#include "Components/VerticalBox.h"
#include "UI/Widgets/SubtitleEntryWidget.h"

USubtitleEntryWidget* USubtitleWidget::AddSubtitle(const FText& Speaker, const FText& Line)
{
	if (!EntryClass || !SubtitleBox) return nullptr;

	USubtitleEntryWidget* Entry = CreateWidget<USubtitleEntryWidget>(this, EntryClass);
	if (Entry)
	{
		Entry->SetupEntry(Speaker, Line);
		SubtitleBox->AddChildToVerticalBox(Entry);
	}
	return Entry;
}

void USubtitleWidget::CullOldEntries() const
{
	if (!SubtitleBox) return;
	while (SubtitleBox->GetChildrenCount() > MaxEntryAmount)
	{
		SubtitleBox->RemoveChildAt(0);
	}
}

void USubtitleWidget::ClearAllSubtitles()
{
	if (!SubtitleBox) return;
	SubtitleBox->ClearChildren();
}
