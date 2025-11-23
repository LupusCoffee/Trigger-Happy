#ifdef WITH_EDITOR
#include "GP4PrototypeEditor/Public/Customizations/AgentAttributeCustomization.h"

#include "Customizations/DataAssetEditorHelpers.h"
#include "Systems/AttributeSystem/AgentData.h"

#include <functional>
#include "DetailWidgetRow.h"
#include "GameplayTagsManager.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"

namespace
{
	static bool GSortTagAsc = true;
	static bool GSortValueAsc = true;
	static bool GSortTypeAsc = true;
	static bool GSortRoundAsc = true;
	static bool GSortMaxAsc = true;
	static bool GSortMaxValueAsc = true;
}

TSharedRef<IPropertyTypeCustomization> AgentAttributeCustomization::MakeInstance()
{
	return MakeShareable(new AgentAttributeCustomization());
}

void AgentAttributeCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> TagHandle   = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAgentAttribute, AttributeTag));
	TSharedPtr<IPropertyHandle> ValueHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAgentAttribute, BaseValue));
	TSharedPtr<IPropertyHandle> TypeHandle  = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAgentAttribute, NumericType));
	TSharedPtr<IPropertyHandle> RoundHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAgentAttribute, RoundingMode));
	TSharedPtr<IPropertyHandle> MaxOnHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAgentAttribute, ClampMode));
	TSharedPtr<IPropertyHandle> MaxValHandle= StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAgentAttribute, ClampValue));
	
	// NEW: ensure we can refresh details panel when clamp mode changes so the clamp value field enables immediately
	TSharedPtr<IPropertyUtilities> PropUtils = CustomizationUtils.GetPropertyUtilities();
	GP4EditorHelpers::RegisterRefreshOnChange(MaxOnHandle, PropUtils);

	if (!TagHandle.IsValid() || !ValueHandle.IsValid() || !RoundHandle.IsValid() || !TypeHandle.IsValid() || !MaxOnHandle.IsValid() || !MaxValHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("One or more property handles are invalid in AgentAttributeCustomization!"));
		return;
	}

	// Helper: fetch DevComment (moved to helper)
	auto GetDevCommentText = [TagHandle]() -> FText
	{
		return GP4EditorHelpers::GetDevCommentForTagHandle(TagHandle, FText::FromString(TEXT("Select an Attribute Tag to see its description")));
	};

	// Derive filter from property meta if present (meta=(Categories=...))
	FString FilterString = GP4EditorHelpers::GetGameplayTagFilterFromMeta(TagHandle, TEXT("Attribute"));

	auto BuildRowContent = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
		// Value on the left, fixed minimum width (moved to helper with Integer rounding support)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0, 0, 8, 0))
		[
			GP4EditorHelpers::MakeFloatNumericEntryForProperty(ValueHandle, TypeHandle, /*MinWidth*/100.f, /*FloatMaxFractionalDigits*/4)
		]
		// Numeric type enum next
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0, 0, 8, 0))
		[
			SNew(SBox)
			.MinDesiredWidth(120.f)
			[
				TypeHandle->CreatePropertyValueWidget()
			]
		]
		// Rounding enum
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0, 0, 8, 0))
		[
			SNew(SBox)
			.MinDesiredWidth(160.f)
			[
				RoundHandle->CreatePropertyValueWidget()
			]
		]
		// Clamp mode
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0, 0, 8, 0))
		[
			SNew(SBox)
			.MinDesiredWidth(110.f)
			[
				MaxOnHandle->CreatePropertyValueWidget()
			]
		]
		// Clamp value (use property widget to respect EditCondition)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0, 0, 8, 0))
		[
			SNew(SBox)
			.MinDesiredWidth(110.f)
			[
				MaxValHandle->CreatePropertyValueWidget()
			]
		]
		// Tag picker on the right, fills
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			GP4EditorHelpers::MakeGameplayTagCombo(TagHandle, FilterString)
		]
		// Delete button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(FMargin(6, 0, 0, 0))
		[
			GP4EditorHelpers::MakeDeleteArrayElementButton(StructPropertyHandle, CustomizationUtils, FText::FromString(TEXT("Remove this attribute")))
		]
		;
	};

	// Build a header row composed of sort buttons aligned to the same columns as the content
	auto BuildSortHeader = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
		// Value column
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAgentAttribute>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortValueAsc,
				100.f,
				FText::FromString(TEXT("Value")),
				FText::FromString(TEXT("Sort by Base Value")),
				[](const FAgentAttribute& A, const FAgentAttribute& B){ return A.BaseValue < B.BaseValue; },
				FText::FromString(TEXT("Sort Agent Attributes (Value)"))
			)
		]
		// Type column
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAgentAttribute>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortTypeAsc,
				120.f,
				FText::FromString(TEXT("Type")),
				FText::FromString(TEXT("Sort by Numeric Type")),
				[](const FAgentAttribute& A, const FAgentAttribute& B){ return (int32)A.NumericType < (int32)B.NumericType; },
				FText::FromString(TEXT("Sort Agent Attributes (Type)"))
			)
		]
		// Rounding column
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAgentAttribute>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortRoundAsc,
				160.f,
				FText::FromString(TEXT("Rounding")),
				FText::FromString(TEXT("Sort by Rounding Mode")),
				[](const FAgentAttribute& A, const FAgentAttribute& B){ return (int32)A.RoundingMode < (int32)B.RoundingMode; },
				FText::FromString(TEXT("Sort Agent Attributes (Rounding)"))
			)
		]
		// Clamp mode column
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAgentAttribute>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortMaxAsc,
				110.f,
				FText::FromString(TEXT("Clamp")),
				FText::FromString(TEXT("Sort by Clamp Mode")),
				[](const FAgentAttribute& A, const FAgentAttribute& B){ return (int32)A.ClampMode < (int32)B.ClampMode; },
				FText::FromString(TEXT("Sort Agent Attributes (Clamp Mode)"))
			)
		]
		// Clamp Value column
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAgentAttribute>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortMaxValueAsc,
				110.f,
				FText::FromString(TEXT("Clamp Value")),
				FText::FromString(TEXT("Sort by Clamp Value")),
				[](const FAgentAttribute& A, const FAgentAttribute& B){ return A.ClampValue < B.ClampValue; },
				FText::FromString(TEXT("Sort Agent Attributes (Clamp Value)"))
			)
		]
		// Attribute tag column (fills)
		+ SHorizontalBox::Slot().FillWidth(1.f)
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAgentAttribute>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortTagAsc,
				220.f,
				FText::FromString(TEXT("Attribute")),
				FText::FromString(TEXT("Sort by Attribute tag")),
				[](const FAgentAttribute& A, const FAgentAttribute& B)
				{
					const bool AValid = A.AttributeTag.IsValid();
					const bool BValid = B.AttributeTag.IsValid();
					if (AValid != BValid) return AValid; // invalids sort last when ascending
					if (!AValid || !BValid) return false;
					return A.AttributeTag.GetTagName().LexicalLess(B.AttributeTag.GetTagName());
				},
				FText::FromString(TEXT("Sort Agent Attributes (Tag)"))
			)
		]
		// Add button on far right
		+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right).Padding(FMargin(8,0,0,0))
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Add All Missing Attributes")))
			.ToolTipText(FText::FromString(TEXT("Add all missing leaf attribute tags as new rows.")))
			.OnClicked_Lambda([StructPropertyHandle, &CustomizationUtils]() -> FReply
			{
				TSharedPtr<IPropertyHandle> ParentHandle = StructPropertyHandle->GetParentHandle();
				if (!ParentHandle.IsValid()) return FReply::Handled();
				TSharedPtr<IPropertyHandleArray> ArrayHandle = ParentHandle->AsArray();
				if (!ArrayHandle.IsValid()) return FReply::Handled();

				TSharedPtr<IPropertyUtilities> Utils = CustomizationUtils.GetPropertyUtilities();
				GP4EditorHelpers::AddAllMissingLeafTags(
					ArrayHandle,
					GET_MEMBER_NAME_CHECKED(FAgentAttribute, AttributeTag),
					TEXT("Attribute"),
					Utils);
				return FReply::Handled();
			})
		];
	};

	// Determine if this is the first element of an array to draw the sort header above it
	const int32 IndexInArray = StructPropertyHandle->GetIndexInArray();
	const bool bShowHeader = (IndexInArray == 0);

	HeaderRow
	.WholeRowContent()
	[
		SNew(SVerticalBox)
		// Show the sort header (no static labels) only for the first row
		+ SVerticalBox::Slot().AutoHeight()
		[
			bShowHeader ? BuildSortHeader() : StaticCastSharedRef<SWidget>(SNullWidget::NullWidget)
		]
		// Data row content
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
			.ToolTipText_Lambda(GetDevCommentText)
			.Padding(FMargin(0))
			[
				BuildRowContent()
			]
		]
	];
}

void AgentAttributeCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> /*StructPropertyHandle*/,
	IDetailChildrenBuilder& /*ChildBuilder*/,
	IPropertyTypeCustomizationUtils& /*CustomizationUtils*/)
{
	// Flat row; no children.
}

#endif // WITH_EDITOR
