#if WITH_EDITOR

#include "GP4PrototypeEditor/Public/Customizations/SubtitleNodeCustomization.h"

#include "Customizations/DataAssetEditorHelpers.h"
#include "Customizations/EditorUISettings.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "IPropertyUtilities.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "ScopedTransaction.h"

// Data type we customize
#include "Systems/SubtitleSystem/SubtitleScriptData.h"

namespace
{
	static bool GSortEventAsc    = true;
	static bool GSortSoundAsc    = true;
	static bool GSortSpeakerAsc  = true;
	static bool GSortLineAsc     = true;
	static bool GSortDurationAsc = true;
}

TSharedRef<IPropertyTypeCustomization> SubtitleNodeCustomization::MakeInstance()
{
	return MakeShareable(new SubtitleNodeCustomization());
}

void SubtitleNodeCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> EventEnumHandle      = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSubtitleNode, Event));
	TSharedPtr<IPropertyHandle> EventCustomHandle    = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSubtitleNode, EventCustom));
	TSharedPtr<IPropertyHandle> SoundHandle          = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSubtitleNode, SpecificSound));
	TSharedPtr<IPropertyHandle> SpeakerEnumHandle    = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSubtitleNode, SpeakerType));
	TSharedPtr<IPropertyHandle> SpeakerCustomHandle  = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSubtitleNode, SpeakerCustomText));
	TSharedPtr<IPropertyHandle> LineHandle           = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSubtitleNode, Line));
	TSharedPtr<IPropertyHandle> DurationHandle       = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSubtitleNode, DurationOverride));

	// Ensure UI refreshes when enum value flips to Custom so the below field appears immediately
	if (TSharedPtr<IPropertyUtilities> Utils = CustomizationUtils.GetPropertyUtilities())
	{
		GP4EditorHelpers::RegisterRefreshOnChange(EventEnumHandle, Utils);
		GP4EditorHelpers::RegisterRefreshOnChange(SpeakerEnumHandle, Utils);
	}

	// Detect owning data asset to read overrides
	TWeakObjectPtr<USubtitleScriptData> OwnerAsset;
	{
		TArray<UObject*> Outers; StructPropertyHandle->GetOuterObjects(Outers);
		for (UObject* Obj : Outers)
		{
			if (USubtitleScriptData* AsData = Cast<USubtitleScriptData>(Obj))
			{
				OwnerAsset = AsData;
				break;
			}
		}
	}

	// Build a compact row: Event(enum+custom or override label) | Sound | Duration | Speaker(enum+custom or override label) | Line | Delete
	auto BuildRowContent = [&]() -> TSharedRef<SWidget>
	{
		const auto MakeEventWidget = [&]() -> TSharedRef<SWidget>
		{
			return SNew(SBox)
			.WidthOverride(GP4EditorUI::EnumPickerWidth)
			[
				SNew(SVerticalBox)
				// Override preview
				+ SVerticalBox::Slot().AutoHeight()
				[
					GP4EditorHelpers::MakeOverrideEnumPreview(
						[OwnerAsset]()
						{
							return (OwnerAsset.IsValid() && OwnerAsset->EventOverride != EventType::None);
						},
						[OwnerAsset]()
						{
							if (!OwnerAsset.IsValid()) return FText::GetEmpty();
							if (OwnerAsset->EventOverride == EventType::Custom)
							{
								return FText::FromString(TEXT("Custom"));
							}
							if (UEnum* EnumDef = StaticEnum<EventType>())
							{
								return EnumDef->GetDisplayNameTextByValue(static_cast<int64>(OwnerAsset->EventOverride));
							}
							return FText::GetEmpty();
						},
						[OwnerAsset]()
						{
							return (OwnerAsset.IsValid() && OwnerAsset->EventOverride == EventType::Custom);
						},
						[OwnerAsset]()
						{
							return OwnerAsset.IsValid()
								? FText::FromString(OwnerAsset->EventOverrideCustom.IsNone() ? TEXT("") : OwnerAsset->EventOverrideCustom.ToString())
								: FText::GetEmpty();
						},
						/*WidthOverride*/GP4EditorUI::EnumPickerWidth,
						/*CustomLeftIndent*/GP4EditorUI::CustomLeftIndent)
				]
				// Enum + custom input variant (visible when no global override)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox)
					.Visibility_Lambda([OwnerAsset]()
					{
						return (OwnerAsset.IsValid() && OwnerAsset->EventOverride != EventType::None) ? EVisibility::Collapsed : EVisibility::Visible;
					})
					[
						GP4EditorHelpers::MakeEnumWithCustomBelow(
							EventEnumHandle,
							EventCustomHandle,
							/*WidthOverride*/GP4EditorUI::EnumPickerWidth,
							TEXT("Custom"),
							/*CustomLeftIndent*/GP4EditorUI::CustomLeftIndent)
					]
				]
			];
		};

		const auto MakeDurationWidget = [&]() -> TSharedRef<SWidget>
		{
			return SNew(SBox)
			.WidthOverride(GP4EditorUI::DurationColumnWidth)
			[
				SNew(SVerticalBox)
				// Override display when global duration override is set
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SVerticalBox)
					.Visibility_Lambda([OwnerAsset]()
					{
						return (OwnerAsset.IsValid() && OwnerAsset->GlobalDurationOverride > 0.f) ? EVisibility::Visible : EVisibility::Collapsed;
					})
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Override:")))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.7f,0.7f,0.7f)))
						.Font(FAppStyle::Get().GetFontStyle("SmallFont"))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0,2,0,0))
					[
						SNew(SNumericEntryBox<float>)
						.IsEnabled(false)
						.MinFractionalDigits(0)
						.MaxFractionalDigits(3)
						.Value_Lambda([OwnerAsset]() -> TOptional<float>
						{
							return OwnerAsset.IsValid() ? OwnerAsset->GlobalDurationOverride : 0.f;
						})
					]
				]
				// Per-node optional float visible when no global override
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox)
					.Visibility_Lambda([OwnerAsset]()
					{
						return (OwnerAsset.IsValid() && OwnerAsset->GlobalDurationOverride > 0.f) ? EVisibility::Collapsed : EVisibility::Visible;
					})
					[
						GP4EditorHelpers::MakeOptionalOverrideFloat(
							DurationHandle,
							/*ColumnWidth*/GP4EditorUI::DurationColumnWidth,
							/*NumericWidth*/GP4EditorUI::DurationNumericWidth,
							FText::FromString(TEXT("Override")),
							FText::FromString(TEXT("Auto from sound length")),
							/*FloatMaxFractionalDigits*/3,
							/*GapPadding*/GP4EditorUI::DurationGapPadding)
					]
				]
			];
		};

		const auto MakeSpeakerWidget = [&]() -> TSharedRef<SWidget>
		{
			return SNew(SBox)
			.WidthOverride(GP4EditorUI::EnumPickerWidth)
			[
				SNew(SVerticalBox)
				// Override preview
				+ SVerticalBox::Slot().AutoHeight()
				[
					GP4EditorHelpers::MakeOverrideEnumPreview(
						[OwnerAsset]()
						{
							return (OwnerAsset.IsValid() && OwnerAsset->SpeakerOverride != Speaker::None);
						},
						[OwnerAsset]()
						{
							if (!OwnerAsset.IsValid()) return FText::GetEmpty();
							if (OwnerAsset->SpeakerOverride == Speaker::Custom)
							{
								return FText::FromString(TEXT("Custom"));
							}
							if (UEnum* EnumDef = StaticEnum<Speaker>())
							{
								return EnumDef->GetDisplayNameTextByValue(static_cast<int64>(OwnerAsset->SpeakerOverride));
							}
							return FText::GetEmpty();
						},
						[OwnerAsset]()
						{
							return (OwnerAsset.IsValid() && OwnerAsset->SpeakerOverride == Speaker::Custom);
						},
						[OwnerAsset]()
						{
							return OwnerAsset.IsValid() ? OwnerAsset->SpeakerOverrideCustomText : FText::GetEmpty();
						},
						/*WidthOverride*/GP4EditorUI::EnumPickerWidth,
						/*CustomLeftIndent*/GP4EditorUI::CustomLeftIndent)
				]
				// Enum + custom input variant (visible when no global override)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBox)
					.Visibility_Lambda([OwnerAsset]()
					{
						return (OwnerAsset.IsValid() && OwnerAsset->SpeakerOverride != Speaker::None) ? EVisibility::Collapsed : EVisibility::Visible;
					})
					[
						GP4EditorHelpers::MakeEnumWithCustomBelow(
							SpeakerEnumHandle,
							SpeakerCustomHandle,
							/*WidthOverride*/GP4EditorUI::EnumPickerWidth,
							TEXT("Custom"),
							/*CustomLeftIndent*/GP4EditorUI::CustomLeftIndent)
					]
				]
			];
		};

		return SNew(SHorizontalBox)
		// Event (Enum with custom below or override label)
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,0)).VAlign(VAlign_Center)
		[
			MakeEventWidget()
		]
		// Sound (USoundBase* picker)
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,0))
		[
			GP4EditorHelpers::MakePropertyWidgetFixed(SoundHandle, /*WidthOverride*/GP4EditorUI::SoundPickerWidth)
		]
		// DurationOverride (optional float or global label)
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,0)).VAlign(VAlign_Center)
		[
			MakeDurationWidget()
		]
		// Speaker (Enum with custom below or override label)
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,0)).VAlign(VAlign_Center)
		[
			MakeSpeakerWidget()
		]
		// Line (FText, MultiLine via meta; allow to stretch)
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(FMargin(0,0,8,0)).VAlign(VAlign_Center)
		[
			GP4EditorHelpers::MakePropertyWidgetFixed(LineHandle, /*WidthOverride*/0.f)
		]
		// Delete button if an array element
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(6,0,0,0))
		[
			GP4EditorHelpers::MakeDeleteArrayElementButton(StructPropertyHandle, CustomizationUtils, FText::FromString(TEXT("Remove subtitle line")))
		];
	};

	// Build a header row with sortable buttons aligned to content columns
	auto BuildSortHeader = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
		// Event
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FSubtitleNode>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortEventAsc,
				GP4EditorUI::EnumPickerWidth,
				FText::FromString(TEXT("Event")),
				FText::FromString(TEXT("Sort by Event")),
				[OwnerAsset](const FSubtitleNode& A, const FSubtitleNode& B)
				{
					const FName AN = A.GetSortableEventName(OwnerAsset.Get());
					const FName BN = B.GetSortableEventName(OwnerAsset.Get());
					return AN.LexicalLess(BN);
				},
				FText::FromString(TEXT("Sort Subtitles (Event)"))
			)
		]
		// Sound
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FSubtitleNode>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortSoundAsc,
				GP4EditorUI::SoundPickerWidth,
				FText::FromString(TEXT("Sound")),
				FText::FromString(TEXT("Sort by Sound asset name (empty first/last)")),
				[](const FSubtitleNode& A, const FSubtitleNode& B)
				{
					const FName AN = (A.SpecificSound ? A.SpecificSound->GetFName() : NAME_None);
					const FName BN = (B.SpecificSound ? B.SpecificSound->GetFName() : NAME_None);
					return AN.LexicalLess(BN);
				},
				FText::FromString(TEXT("Sort Subtitles (Sound)"))
			)
		]
		// Duration
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FSubtitleNode>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortDurationAsc,
				GP4EditorUI::DurationColumnWidth,
				FText::FromString(TEXT("Duration")),
				FText::FromString(TEXT("Sort by DurationOverride")),
				[](const FSubtitleNode& A, const FSubtitleNode& B){ return A.DurationOverride < B.DurationOverride; },
				FText::FromString(TEXT("Sort Subtitles (Duration)"))
			)
		]
		// Speaker
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FSubtitleNode>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortSpeakerAsc,
				GP4EditorUI::EnumPickerWidth,
				FText::FromString(TEXT("Speaker")),
				FText::FromString(TEXT("Sort by Speaker")),
				[OwnerAsset](const FSubtitleNode& A, const FSubtitleNode& B)
				{
					const FString AS = A.ResolveSpeakerText(OwnerAsset.Get()).ToString();
					const FString BS = B.ResolveSpeakerText(OwnerAsset.Get()).ToString();
					return AS.Compare(BS) < 0;
				},
				FText::FromString(TEXT("Sort Subtitles (Speaker)"))
			)
		]
		// Line (fills)
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FSubtitleNode>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortLineAsc,
				GP4EditorUI::SortHeaderLineMinWidth,
				FText::FromString(TEXT("Line")),
				FText::FromString(TEXT("Sort by subtitle line text")),
				[](const FSubtitleNode& A, const FSubtitleNode& B){ return A.Line.ToString().Compare(B.Line.ToString()) < 0; },
				FText::FromString(TEXT("Sort Subtitles (Line)"))
			)
		]
		;
	};

	const int32 IndexInArray = StructPropertyHandle->GetIndexInArray();
	const bool bShowHeader = (IndexInArray == 0);

	HeaderRow
	.WholeRowContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			bShowHeader ? BuildSortHeader() : StaticCastSharedRef<SWidget>(SNullWidget::NullWidget)
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
			.Padding(FMargin(0))
			[
				BuildRowContent()
			]
		]
	];
}

void SubtitleNodeCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> /*StructPropertyHandle*/,
	IDetailChildrenBuilder& /*ChildBuilder*/,
	IPropertyTypeCustomizationUtils& /*CustomizationUtils*/)
{
	// One-line customization; no children.
}

#endif // WITH_EDITOR
