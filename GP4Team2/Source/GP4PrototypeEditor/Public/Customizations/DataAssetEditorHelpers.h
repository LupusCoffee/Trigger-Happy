#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IPropertyTypeCustomization.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"

class IPropertyHandle;
class IPropertyTypeCustomizationUtils;
class IPropertyUtilities;
class SWidget;

namespace GP4EditorHelpers
{
	// Ensures the specified FGuid child property is valid (assigns a new Guid if invalid)
	void EnsureGuidOnChild(TSharedRef<IPropertyHandle> StructPropertyHandle, FName GuidChildPropertyName);

	// Register a ForceRefresh on value change for a handle (useful for EditCondition-driven widgets)
	void RegisterRefreshOnChange(TSharedPtr<IPropertyHandle> Handle, TSharedPtr<IPropertyUtilities> PropUtils);

	// A standardized delete button for array elements; collapsed if this struct is not an array element
	TSharedRef<SWidget> MakeDeleteArrayElementButton(TSharedRef<IPropertyHandle> StructPropertyHandle,
		IPropertyTypeCustomizationUtils& CustomizationUtils,
		const FText& Tooltip);

	// A compact gameplay tag combo button bound to a property handle (Categories meta respected if present)
	TSharedRef<SWidget> MakeGameplayTagCombo(TSharedPtr<IPropertyHandle> TagHandle,
		const FString& FilterString = TEXT("Attribute"));

	// Utility: read Categories meta from a GameplayTag property, else return DefaultFilter
	FString GetGameplayTagFilterFromMeta(TSharedPtr<IPropertyHandle> TagHandle, const FString& DefaultFilter = TEXT("Attribute"));

	// Utility: resolve tag DevComment tooltip for the first selected tag on the handle
	FText GetDevCommentForTagHandle(TSharedPtr<IPropertyHandle> TagHandle, const FText& FallbackText);

	// Generic numeric entry for a float property with optional integer rounding governed by a 'NumericType' enum property
	// If NumericTypeHandle is provided and equals EAttributeNumericType::Integer, show 0 fractional digits and round values on commit/change.
	TSharedRef<SWidget> MakeFloatNumericEntryForProperty(TSharedPtr<IPropertyHandle> ValueHandle,
		TSharedPtr<IPropertyHandle> NumericTypeHandle,
		float MinDesiredWidth = 100.f,
		int32 FloatMaxFractionalDigits = 4);

	// GameplayTags helpers
	void GatherLeafTagsUnder(const FString& RootPath, TArray<FGameplayTag>& OutLeafTags);

	// Try to set a gameplay tag value on a property handle (string first, raw fallback)
	bool TrySetGameplayTagOnHandle(TSharedPtr<IPropertyHandle> TagHandle, const FGameplayTag& Tag);

	// Add all missing leaf tags (under RootPath) as new array rows and set TagMemberProperty on each new element.
	// ArrayHandle must be a valid AsArray() handle of the parent array.
	void AddAllMissingLeafTags(TSharedPtr<IPropertyHandleArray> ArrayHandle,
		FName TagMemberPropertyName,
		const FString& RootPath,
		TSharedPtr<IPropertyUtilities> PropUtils);

	// Compact wrapper around CreatePropertyValueWidget inside a MinDesiredWidth SBox.
	TSharedRef<SWidget> MakePropertyWidgetBox(TSharedPtr<IPropertyHandle> Handle,
		float MinDesiredWidth = 120.f);

	// Fixed-width wrapper around CreatePropertyValueWidget.
	TSharedRef<SWidget> MakePropertyWidgetFixed(TSharedPtr<IPropertyHandle> Handle,
		float WidthOverride);

	// Format a gameplay tag for display:
	// - Drop leading "Attribute." root if present
	// - Replace '.' separators with " - "
	// - Split camel case and underscores into spaces
	FText FormatGameplayTagForDisplay(const FGameplayTag& Tag);

	// Resolve the first selected gameplay tag on a handle and return a formatted display text.
	// If none, returns FallbackText.
	FText GetFormattedTagFromHandle(TSharedPtr<IPropertyHandle> TagHandle, const FText& FallbackText);

	// NEW: Reusable templated sort button builder for array properties.
	// Usage: MakeSortButtonForArray<FMyStruct>(StructPropertyHandle, CustomizationUtils, bSortAsc, 140.f, TEXT("Label"), TEXT("Tooltip"),
	//          [](const FMyStruct& A, const FMyStruct& B){ return A.Field < B.Field; });
	template<typename T, typename LessPred>
	TSharedRef<SWidget> MakeSortButtonForArray(
		TSharedRef<IPropertyHandle> StructPropertyHandle,
		IPropertyTypeCustomizationUtils& CustomizationUtils,
		bool& bSortAscending,
		float MinDesiredWidth,
		const FText& Label,
		const FText& Tooltip,
		LessPred LessFunc,
		const FText& TransactionText = FText::FromString(TEXT("Sort Items")))
	{
		return SNew(SBox)
		.WidthOverride(MinDesiredWidth)
		[
			SNew(SButton)
			.ContentPadding(FMargin(6,2))
			.Text_Lambda([&bSortAscending, Label]()
			{
				return FText::FromString(bSortAscending ? Label.ToString() + TEXT(" ↑") : Label.ToString() + TEXT(" ↓"));
			})
			.ToolTipText(Tooltip)
			.OnClicked_Lambda([StructPropertyHandle, &CustomizationUtils, &bSortAscending, LessFunc, TransactionText]() -> FReply
			{
				bSortAscending = !bSortAscending;
				TSharedPtr<IPropertyHandle> Parent = StructPropertyHandle->GetParentHandle();
				if (!Parent.IsValid()) return FReply::Handled();
				TArray<void*> RawData; Parent->AccessRawData(RawData);
				FScopedTransaction Tx(TransactionText);
				TArray<UObject*> Outers; Parent->GetOuterObjects(Outers);
				for (UObject* Obj : Outers) { if (Obj) Obj->Modify(); }
				for (void* Data : RawData)
				{
					if (TArray<T>* Arr = static_cast<TArray<T>*>(Data))
					{
						Arr->Sort([&](const T& A, const T& B)
						{
							const bool Less = LessFunc(A, B);
							return bSortAscending ? Less : !Less;
						});
					}
				}
				if (TSharedPtr<IPropertyUtilities> Utils = CustomizationUtils.GetPropertyUtilities()) { Utils->ForceRefresh(); }
				return FReply::Handled();
			})
		];
	}

	// Show an optional float override as a two-row control with fixed column width.
	// Top row: [Checkbox+Label (auto/fixed)] [NumericBox (fixed NumericWidth)]
	// Bottom row: grey hint (wraps within ColumnWidth) visible when value <= 0.
	TSharedRef<SWidget> MakeOptionalOverrideFloat(
		TSharedPtr<IPropertyHandle> ValueHandle,
		float ColumnWidth,
		float NumericWidth,
		const FText& CheckLabel,
		const FText& DisabledNote,
		int32 FloatMaxFractionalDigits = 3,
		float GapPadding = 6.f);

	// Vertical variant: shows a validation note above the field when empty (FName none or FText empty/whitespace)
	TSharedRef<SWidget> MakePropertyWidgetFixedWithValidationNote(
		TSharedPtr<IPropertyHandle> Handle,
		float WidthOverride,
		const FText& InvalidNoteText,
		FLinearColor InvalidColor = FLinearColor(0.85f, 0.2f, 0.2f));

	// NEW: Shows an enum dropdown, and when the enum value equals the named entry (default: "Custom"),
	// renders the provided CustomHandle's input below it. Both controls are constrained to WidthOverride.
	// Useful for patterns like [Enum=Custom] -> [Show Name/Text below].
	TSharedRef<SWidget> MakeEnumWithCustomBelow(
		TSharedPtr<IPropertyHandle> EnumHandle,
		TSharedPtr<IPropertyHandle> CustomHandle,
		float WidthOverride,
		const TCHAR* CustomEntryName = TEXT("Custom"),
		float CustomLeftIndent = 0.f);

	// NEW: A compact override preview showing a grey "Override:" label, a disabled enum-like display,
	// and an optional disabled custom value below. Visibility and content are provided via lambdas.
	TSharedRef<SWidget> MakeOverrideEnumPreview(
		TFunction<bool()> bShowOverride,
		TFunction<FText()> EnumDisplayText,
		TFunction<bool()> bShowCustomBelow,
		TFunction<FText()> CustomDisplayText,
		float WidthOverride,
		float CustomLeftIndent);
}

#endif // WITH_EDITOR
