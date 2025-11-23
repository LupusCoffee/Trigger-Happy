#ifdef WITH_EDITOR

#include "GP4PrototypeEditor/Public/Customizations/AttributeModifierCustomization.h"

#include "Customizations/DataAssetEditorHelpers.h"
#include "DataStructures/AttributeUpgradeDataStructs.h"

#include <functional>
#include "DetailWidgetRow.h"
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
	static bool GSortAttrAsc  = true;
	static bool GSortTypeAsc  = true;
	static bool GSortValueAsc = true;
	static bool GSortDirAsc   = true;
}

TSharedRef<IPropertyTypeCustomization> AttributeModifierCustomization::MakeInstance()
{
	return MakeShareable(new AttributeModifierCustomization());
}

void AttributeModifierCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> TagHandle   = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAttributeModifierEntry, TargetAttribute));
	TSharedPtr<IPropertyHandle> TypeHandle  = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAttributeModifierEntry, Type));
	TSharedPtr<IPropertyHandle> ValueHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAttributeModifierEntry, Value));
	TSharedPtr<IPropertyHandle> DirHandle   = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAttributeModifierEntry, BenefitDirection));

	// Ensure a stable Id exists
	GP4EditorHelpers::EnsureGuidOnChild(StructPropertyHandle, GET_MEMBER_NAME_CHECKED(FAttributeModifierEntry, Id));

	// DevComment tooltip for the picked tag
	auto GetDevCommentText = [TagHandle]() -> FText
	{
		return GP4EditorHelpers::GetDevCommentForTagHandle(TagHandle, FText::FromString(TEXT("Select an Attribute Tag to see its description")));
	};

	// Categories meta respected (fallback to "Attribute")
	FString FilterString = GP4EditorHelpers::GetGameplayTagFilterFromMeta(TagHandle, TEXT("Attribute"));

	// Build a single row of content (Type | Value | Direction | Tag | Delete)
	auto BuildRowContent = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
		// Type
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,0))
		[
			GP4EditorHelpers::MakePropertyWidgetBox(TypeHandle, /*MinDesiredWidth*/140.f)
		]
		// Value
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,0))
		[
			GP4EditorHelpers::MakeFloatNumericEntryForProperty(ValueHandle, /*NumericTypeHandle*/nullptr, /*MinWidth*/120.f, /*FloatMaxFractionalDigits*/4)
		]
		// Direction
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,0))
		[
			GP4EditorHelpers::MakePropertyWidgetBox(DirHandle, /*MinDesiredWidth*/160.f)
		]
		// Attribute picker (fills)
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			GP4EditorHelpers::MakeGameplayTagCombo(TagHandle, FilterString)
		]
		// Delete in arrays
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(6,0,0,0))
		[
			GP4EditorHelpers::MakeDeleteArrayElementButton(StructPropertyHandle, CustomizationUtils, FText::FromString(TEXT("Remove modifier")))
		];
	};

	// Build a header row with sort buttons aligned to columns, plus a global add-all button
	auto BuildSortHeader = [&]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
		// Type
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAttributeModifierEntry>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortTypeAsc,
				140.f,
				FText::FromString(TEXT("Type")),
				FText::FromString(TEXT("Sort by Modifier Type")),
				[](const FAttributeModifierEntry& A, const FAttributeModifierEntry& B){ return (int32)A.Type < (int32)B.Type; },
				FText::FromString(TEXT("Sort Modifiers (Type)"))
			)
		]
		// Value
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAttributeModifierEntry>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortValueAsc,
				120.f,
				FText::FromString(TEXT("Value")),
				FText::FromString(TEXT("Sort by Value")),
				[](const FAttributeModifierEntry& A, const FAttributeModifierEntry& B){ return A.Value < B.Value; },
				FText::FromString(TEXT("Sort Modifiers (Value)"))
			)
		]
		// Direction
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(0,0,8,2))
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAttributeModifierEntry>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortDirAsc,
				160.f,
				FText::FromString(TEXT("Direction")),
				FText::FromString(TEXT("Sort by Benefit Direction")),
				[](const FAttributeModifierEntry& A, const FAttributeModifierEntry& B){ return (int32)A.BenefitDirection < (int32)B.BenefitDirection; },
				FText::FromString(TEXT("Sort Modifiers (Direction)"))
			)
		]
		// Attribute tag (fills)
		+ SHorizontalBox::Slot().FillWidth(1.f)
		[
			GP4EditorHelpers::MakeSortButtonForArray<FAttributeModifierEntry>(
				StructPropertyHandle,
				CustomizationUtils,
				GSortAttrAsc,
				220.f,
				FText::FromString(TEXT("Attribute")),
				FText::FromString(TEXT("Sort by Attribute Tag")),
				[](const FAttributeModifierEntry& A, const FAttributeModifierEntry& B)
				{
					const bool AValid = A.TargetAttribute.IsValid();
					const bool BValid = B.TargetAttribute.IsValid();
					if (AValid != BValid) return AValid; // invalids last when ascending
					if (!AValid || !BValid) return false;
					const FName AN = A.TargetAttribute.GetTagName();
					const FName BN = B.TargetAttribute.GetTagName();
					return AN.LexicalLess(BN);
				},
				FText::FromString(TEXT("Sort Modifiers (Tag)"))
			)
		]
		// Add all missing attribute entries (leaf tags under "Attribute")
		+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right).Padding(FMargin(8,0,0,0))
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Add All Missing Attributes")))
			.ToolTipText(FText::FromString(TEXT("Add all missing leaf attribute tags as new modifier rows.")))
			.OnClicked_Lambda([StructPropertyHandle, &CustomizationUtils]() -> FReply
			{
				if (TSharedPtr<IPropertyHandle> Parent = StructPropertyHandle->GetParentHandle())
				{
					if (TSharedPtr<IPropertyHandleArray> ArrayHandle = Parent->AsArray())
					{
						TSharedPtr<IPropertyUtilities> Utils = CustomizationUtils.GetPropertyUtilities();
						GP4EditorHelpers::AddAllMissingLeafTags(
							ArrayHandle,
							GET_MEMBER_NAME_CHECKED(FAttributeModifierEntry, TargetAttribute),
							TEXT("Attribute"),
							Utils);
					}
				}
				return FReply::Handled();
			})
		];
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
			.ToolTipText_Lambda(GetDevCommentText)
			.Padding(FMargin(0))
			[
				BuildRowContent()
			]
		]
	];
}

void AttributeModifierCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> /*StructPropertyHandle*/,
	IDetailChildrenBuilder& /*ChildBuilder*/,
	IPropertyTypeCustomizationUtils& /*CustomizationUtils*/)
{
	// Flat row; no children.
}

// Optional: expose a factory symbol you can use in your module registration
TSharedRef<IPropertyTypeCustomization> MakeAttributeModifierCustomizationInstance()
{
	return AttributeModifierCustomization::MakeInstance();
}

#endif // WITH_EDITOR
