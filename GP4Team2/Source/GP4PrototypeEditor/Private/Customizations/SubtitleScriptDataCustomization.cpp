#if WITH_EDITOR

#include "Customizations/SubtitleScriptDataCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Customizations/DataAssetEditorHelpers.h"
#include "Customizations/EditorUISettings.h"

#include "Systems/SubtitleSystem/SubtitleScriptData.h"

TSharedRef<IDetailCustomization> FSubtitleScriptDataCustomization::MakeInstance()
{
	return MakeShareable(new FSubtitleScriptDataCustomization());
}

void FSubtitleScriptDataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	IDetailCategoryBuilder& Cat = DetailLayout.EditCategory(TEXT("Overrides"), FText::FromString(TEXT("Overrides")), ECategoryPriority::Important);

	TSharedPtr<IPropertyHandle> EventOverrideHandle         = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(USubtitleScriptData, EventOverride));
	TSharedPtr<IPropertyHandle> EventOverrideCustomHandle   = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(USubtitleScriptData, EventOverrideCustom));
	TSharedPtr<IPropertyHandle> SpeakerOverrideHandle       = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(USubtitleScriptData, SpeakerOverride));
	TSharedPtr<IPropertyHandle> SpeakerOverrideCustomHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(USubtitleScriptData, SpeakerOverrideCustomText));
	TSharedPtr<IPropertyHandle> GlobalDurationHandle        = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(USubtitleScriptData, GlobalDurationOverride));

	// Ensure the panel refreshes so the custom input below shows immediately when selecting Custom
	if (TSharedPtr<IPropertyUtilities> Utils = DetailLayout.GetPropertyUtilities())
	{
		GP4EditorHelpers::RegisterRefreshOnChange(EventOverrideHandle, Utils);
		GP4EditorHelpers::RegisterRefreshOnChange(SpeakerOverrideHandle, Utils);
	}

	// Hide originals from default view (we'll render custom compact rows below)
	DetailLayout.HideProperty(EventOverrideHandle);
	DetailLayout.HideProperty(EventOverrideCustomHandle);
	DetailLayout.HideProperty(SpeakerOverrideHandle);
	DetailLayout.HideProperty(SpeakerOverrideCustomHandle);
	DetailLayout.HideProperty(GlobalDurationHandle);

	Cat.AddCustomRow(FText::FromString(TEXT("Event Override")))
	.NameContent()
	[
		EventOverrideHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(GP4EditorUI::EnumPickerWidth)
	[
			GP4EditorHelpers::MakeEnumWithCustomBelow(EventOverrideHandle, EventOverrideCustomHandle, GP4EditorUI::EnumPickerWidth, TEXT("Custom"), GP4EditorUI::CustomLeftIndent)
	];

	Cat.AddCustomRow(FText::FromString(TEXT("Speaker Override")))
	.NameContent()
	[
		SpeakerOverrideHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(GP4EditorUI::EnumPickerWidth)
	[
			GP4EditorHelpers::MakeEnumWithCustomBelow(SpeakerOverrideHandle, SpeakerOverrideCustomHandle, GP4EditorUI::EnumPickerWidth, TEXT("Custom"), GP4EditorUI::CustomLeftIndent)
	];

	Cat.AddCustomRow(FText::FromString(TEXT("Global Duration")))
	.NameContent()
	[
		GlobalDurationHandle->CreatePropertyNameWidget(FText::FromString(TEXT("Global Duration Override")))
	]
	.ValueContent()
	.MinDesiredWidth(GP4EditorUI::DurationColumnWidth)
	[
		GP4EditorHelpers::MakeOptionalOverrideFloat(
			GlobalDurationHandle,
			/*ColumnWidth*/GP4EditorUI::DurationColumnWidth,
			/*NumericWidth*/GP4EditorUI::DurationNumericWidth,
			FText::FromString(TEXT("Override")),
			FText::FromString(TEXT("Auto from per-node or sound length")),
			/*FloatMaxFractionalDigits*/3)
	];
}

#endif // WITH_EDITOR
