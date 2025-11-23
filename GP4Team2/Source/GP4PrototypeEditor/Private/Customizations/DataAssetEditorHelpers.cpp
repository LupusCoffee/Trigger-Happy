#if WITH_EDITOR

#include "Customizations/DataAssetEditorHelpers.h"
#include "PropertyHandle.h"
#include "IPropertyUtilities.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Styling/AppStyle.h"
#include "SGameplayTagPicker.h"
#include "GameplayTagsManager.h"
#include "Misc/DefinePrivateMemberPtr.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "UObject/UnrealType.h"
#include "Customizations/EditorUISettings.h"

UE_DEFINE_PRIVATE_MEMBER_PTR(void(bool, bool),
	SGameplayTagPickerSetTagTreeItemExpansion,
	SGameplayTagPicker,
	SetTagTreeItemExpansion);

namespace GP4EditorHelpers
{
	void EnsureGuidOnChild(TSharedRef<IPropertyHandle> StructPropertyHandle, FName GuidChildPropertyName)
	{
		if (TSharedPtr<IPropertyHandle> IdHandle = StructPropertyHandle->GetChildHandle(GuidChildPropertyName))
		{
			void* DataPtr = nullptr;
			if (IdHandle->GetValueData(DataPtr) == FPropertyAccess::Success && DataPtr)
			{
				FGuid* GuidPtr = static_cast<FGuid*>(DataPtr);
				if (GuidPtr && !GuidPtr->IsValid())
				{
					*GuidPtr = FGuid::NewGuid();
				}
			}
		}
	}

	void RegisterRefreshOnChange(TSharedPtr<IPropertyHandle> Handle, TSharedPtr<IPropertyUtilities> PropUtils)
	{
		if (Handle.IsValid() && PropUtils.IsValid())
		{
			Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([PropUtils]() { PropUtils->ForceRefresh(); }));
		}
	}

	TSharedRef<SWidget> MakeDeleteArrayElementButton(TSharedRef<IPropertyHandle> StructPropertyHandle,
		IPropertyTypeCustomizationUtils& CustomizationUtils,
		const FText& Tooltip)
	{
		const int32 RowIndex = StructPropertyHandle->GetIndexInArray();
		const bool bIsArrayElem = (RowIndex != INDEX_NONE);

		return SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
			.ContentPadding(FMargin(6, 2))
			.ToolTipText(Tooltip)
			.Visibility(bIsArrayElem ? EVisibility::Visible : EVisibility::Collapsed)
			.OnClicked_Lambda([StructPropertyHandle, &CustomizationUtils, RowIndex]() -> FReply
			{
				if (RowIndex != INDEX_NONE)
				{
					if (TSharedPtr<IPropertyHandle> Parent = StructPropertyHandle->GetParentHandle())
					{
						if (TSharedPtr<IPropertyHandleArray> Arr = Parent->AsArray())
						{
							Arr->DeleteItem(RowIndex);
							if (TSharedPtr<IPropertyUtilities> Utils = CustomizationUtils.GetPropertyUtilities())
							{
								Utils->ForceRefresh();
							}
						}
					}
				}
				return FReply::Handled();
			})
			[
				SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Delete"))
			];
	}

	// Split "MaxCharges" -> "Max Charges"
	static FString SplitCamelCase(const FString& In)
	{
		FString Out;
		Out.Reserve(In.Len() + In.Len() / 3);
		for (int32 i = 0; i < In.Len(); ++i)
		{
			const TCHAR C = In[i];
			const bool bIsUpper = FChar::IsUpper(C);
			const bool bIsAlphaNum = FChar::IsAlnum(C);
			if (i > 0)
			{
				const TCHAR Prev = In[i - 1];
				const bool bPrevIsLower = FChar::IsLower(Prev);
				const bool bPrevIsAlphaNum = FChar::IsAlnum(Prev);
				if (bIsUpper && bPrevIsLower && bPrevIsAlphaNum)
				{
					Out.AppendChar(' ');
				}
			}
			Out.AppendChar(bIsAlphaNum ? C : ' ');
		}
		Out.TrimStartAndEndInline();
		return Out.Replace(TEXT("  "), TEXT(" "));
	}

	FText FormatGameplayTagForDisplay(const FGameplayTag& Tag)
	{
		if (!Tag.IsValid())
		{
			return FText::GetEmpty();
		}

		TArray<FString> Parts;
		Tag.ToString().ParseIntoArray(Parts, TEXT("."), true);

		int32 StartIdx = 0;
		if (Parts.Num() > 0 && Parts[0].Equals(TEXT("Attribute"), ESearchCase::IgnoreCase))
		{
			StartIdx = 1; // drop "Attribute"
		}

		TArray<FString> PrettyParts;
		for (int32 i = StartIdx; i < Parts.Num(); ++i)
		{
			FString P = Parts[i];
			P.ReplaceInline(TEXT("_"), TEXT(" "));
			P = SplitCamelCase(P);
			PrettyParts.Add(MoveTemp(P));
		}

		if (PrettyParts.Num() == 0)
		{
			// Fallback to raw name if we had nothing after "Attribute"
			return FText::FromName(Tag.GetTagName());
		}

		return FText::FromString(FString::Join(PrettyParts, TEXT(" - ")));
	}

	FText GetFormattedTagFromHandle(TSharedPtr<IPropertyHandle> TagHandle, const FText& FallbackText)
	{
		TArray<FGameplayTagContainer> Containers;
		if (TagHandle.IsValid()
			&& SGameplayTagPicker::GetEditableTagContainersFromPropertyHandle(TagHandle.ToSharedRef(), Containers)
			&& Containers.Num() > 0 && Containers[0].Num() > 0)
		{
			const FGameplayTag Tag = Containers[0].First();
			return FormatGameplayTagForDisplay(Tag);
		}
		return FallbackText;
	}

	TSharedRef<SWidget> MakeGameplayTagCombo(TSharedPtr<IPropertyHandle> TagHandle, const FString& FilterString)
	{
		TSharedPtr<SComboButton> TagCombo;

		return SNew(SBox)
		.MinDesiredWidth(GP4EditorUI::TagComboMinWidth)
		[
			SAssignNew(TagCombo, SComboButton)
			.ButtonContent()
			[
				// Show the formatted tag text (e.g., "Dash - Max Charges") or a fallback prompt
				SNew(STextBlock)
				.Text_Lambda([TagHandle]()
				{
					return GetFormattedTagFromHandle(TagHandle, FText::FromString(TEXT("Select Attribute")));
				})
			]
			.OnGetMenuContent_Lambda([TagHandle, TagCombo, FilterString]()
			{
				TSharedRef<SGameplayTagPicker> Picker = SNew(SGameplayTagPicker)
					.PropertyHandle(TagHandle)
					.MultiSelect(false)
					.ShowMenuItems(false)
					.Filter(FilterString)
					.MaxHeight(400.0f)
					.Padding(2.0f)
					.OnTagChanged_Lambda([TagCombo](const TArray<FGameplayTagContainer>&)
					{
						if (TagCombo.IsValid()) { TagCombo->SetIsOpen(false); }
					});

				if (SGameplayTagPickerSetTagTreeItemExpansion)
				{
					(Picker.Get().*SGameplayTagPickerSetTagTreeItemExpansion)(true, true);
				}

				return SNew(SBox).WidthOverride(GP4EditorUI::TagPickerMenuWidth).HeightOverride(GP4EditorUI::TagPickerMenuHeight)[ Picker ];
			})
		];
	}

	FString GetGameplayTagFilterFromMeta(TSharedPtr<IPropertyHandle> TagHandle, const FString& DefaultFilter)
	{
		if (TagHandle.IsValid())
		{
			if (const FProperty* TagProp = TagHandle->GetProperty())
			{
				if (TagProp->HasMetaData(TEXT("Categories")))
				{
					const FString Meta = TagProp->GetMetaData(TEXT("Categories"));
					if (!Meta.IsEmpty())
					{
						return Meta;
					}
				}
			}
		}
		return DefaultFilter;
	}

	FText GetDevCommentForTagHandle(TSharedPtr<IPropertyHandle> TagHandle, const FText& FallbackText)
	{
		TArray<FGameplayTagContainer> Containers;
		if (SGameplayTagPicker::GetEditableTagContainersFromPropertyHandle(TagHandle.ToSharedRef(), Containers)
			&& Containers.Num() > 0 && Containers[0].Num() > 0)
		{
			const FGameplayTag Tag = Containers[0].First();
#if WITH_EDITOR
			FString Comment;
			FName FirstSource;
			bool bExplicit = false, bRestricted = false, bAllowNonRestrictedChildren = true;
			if (UGameplayTagsManager::Get().GetTagEditorData(Tag.GetTagName(), Comment, FirstSource, bExplicit, bRestricted, bAllowNonRestrictedChildren))
			{
				if (!Comment.IsEmpty())
				{
					return FText::FromString(Comment);
				}
			}
#endif
			return FText::FromName(Tag.GetTagName());
		}
		return FallbackText;
	}

	TSharedRef<SWidget> MakeFloatNumericEntryForProperty(TSharedPtr<IPropertyHandle> ValueHandle,
		TSharedPtr<IPropertyHandle> NumericTypeHandle,
		float MinDesiredWidth,
		int32 FloatMaxFractionalDigits)
	{
		return SNew(SBox)
		.MinDesiredWidth(MinDesiredWidth)
		[
			SNew(SNumericEntryBox<float>)
			.AllowSpin(false)
			.MinFractionalDigits_Lambda([NumericTypeHandle]() -> int32
			{
				if (!NumericTypeHandle.IsValid()) return 0;
				int32 Raw = 0; NumericTypeHandle->GetValue(Raw);
				return (Raw == 1 /* EAttributeNumericType::Integer */) ? 0 : 0;
			})
			.MaxFractionalDigits_Lambda([NumericTypeHandle, FloatMaxFractionalDigits]() -> int32
			{
				if (!NumericTypeHandle.IsValid()) return FloatMaxFractionalDigits;
				int32 Raw = 0; NumericTypeHandle->GetValue(Raw);
				return (Raw == 1 /* EAttributeNumericType::Integer */) ? 0 : FloatMaxFractionalDigits;
			})
			.Value_Lambda([ValueHandle]() -> TOptional<float>
			{
				float V = 0.f;
				if (ValueHandle.IsValid() && ValueHandle->GetValue(V) == FPropertyAccess::Success)
				{
					return V;
				}
				return TOptional<float>{};
			})
			.OnValueChanged_Lambda([ValueHandle, NumericTypeHandle](float NewValue)
			{
				if (NumericTypeHandle.IsValid())
				{
					int32 Raw = 0; NumericTypeHandle->GetValue(Raw);
					if (Raw == 1 /* Integer */)
					{
						NewValue = FMath::RoundToFloat(NewValue);
					}
				}
				if (ValueHandle.IsValid())
				{
					ValueHandle->SetValue(NewValue);
				}
			})
			.OnValueCommitted_Lambda([ValueHandle, NumericTypeHandle](float NewValue, ETextCommit::Type)
			{
				if (NumericTypeHandle.IsValid())
				{
					int32 Raw = 0; NumericTypeHandle->GetValue(Raw);
					if (Raw == 1 /* Integer */)
					{
						NewValue = FMath::RoundToFloat(NewValue);
					}
				}
				if (ValueHandle.IsValid())
				{
					ValueHandle->SetValue(NewValue);
				}
			})
		];
	}

	TSharedRef<SWidget> MakeOptionalOverrideFloat(
		TSharedPtr<IPropertyHandle> ValueHandle,
		float ColumnWidth,
		float NumericWidth,
		const FText& CheckLabel,
		const FText& DisabledNote,
		int32 FloatMaxFractionalDigits,
		float GapPadding)
	{
		const float LeftWidth = FMath::Max(0.f, ColumnWidth - NumericWidth - GapPadding);
		return SNew(SVerticalBox)
		// Top row: checkbox + numeric entry with fixed widths
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox)
			.WidthOverride(ColumnWidth)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(0,0,GapPadding,0))
				[
					SNew(SBox).WidthOverride(LeftWidth)
					[
						SNew(SCheckBox)
						.IsChecked_Lambda([ValueHandle]()
						{
							float V = 0.f; if (ValueHandle.IsValid() && ValueHandle->GetValue(V) == FPropertyAccess::Success) { }
							return (V > 0.f) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([ValueHandle](ECheckBoxState NewState)
						{
							if (!ValueHandle.IsValid()) return;
							if (NewState == ECheckBoxState::Checked)
							{
								// Use an internal sensible default when enabling override
								ValueHandle->SetValue(1.0f);
							}
							else
							{
								ValueHandle->SetValue(0.f);
							}
						})
						[
							SNew(STextBlock).Text(CheckLabel)
						]
					]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(NumericWidth)
					[
						SNew(SNumericEntryBox<float>)
						.AllowSpin(false)
						.MinFractionalDigits(0)
						.MaxFractionalDigits(FloatMaxFractionalDigits)
						.IsEnabled_Lambda([ValueHandle]()
						{
							float V = 0.f; if (ValueHandle.IsValid() && ValueHandle->GetValue(V) == FPropertyAccess::Success) { }
							return V > 0.f;
						})
						.Value_Lambda([ValueHandle]() -> TOptional<float>
						{
							float V = 0.f;
							if (ValueHandle.IsValid() && ValueHandle->GetValue(V) == FPropertyAccess::Success)
							{
								return V;
							}
							return TOptional<float>{};
						})
						.OnValueChanged_Lambda([ValueHandle](float NewValue)
						{
							if (ValueHandle.IsValid()) { ValueHandle->SetValue(FMath::Max(0.f, NewValue)); }
						})
						.OnValueCommitted_Lambda([ValueHandle](float NewValue, ETextCommit::Type)
						{
							if (ValueHandle.IsValid()) { ValueHandle->SetValue(FMath::Max(0.f, NewValue)); }
						})
					]
				]
			]
		]
		// Bottom row: grey note, wraps under the full column width when disabled
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0,2,0,0))
		[
			SNew(SBox)
			.WidthOverride(ColumnWidth)
			[
				SNew(STextBlock)
				.Text(DisabledNote)
				.AutoWrapText(true)
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f,0.7f,0.7f)))
				.Font(FAppStyle::Get().GetFontStyle("SmallFont"))
				.Visibility_Lambda([ValueHandle]()
				{
					float V = 0.f; if (ValueHandle.IsValid() && ValueHandle->GetValue(V) == FPropertyAccess::Success) { }
					return (V <= 0.f) ? EVisibility::Visible : EVisibility::Collapsed;
				})
			]
		]
		;
	}

	void GatherLeafTagsUnder(const FString& RootPath, TArray<FGameplayTag>& OutLeafTags)
	{
		OutLeafTags.Reset();

		TArray<TSharedPtr<FGameplayTagNode>> TagNodes;
		UGameplayTagsManager::Get().GetFilteredGameplayRootTags(*RootPath, TagNodes);

		TFunction<void(const TSharedPtr<FGameplayTagNode>&)> GatherLeafTags;
		GatherLeafTags = [&OutLeafTags, &GatherLeafTags](const TSharedPtr<FGameplayTagNode>& Node)
		{
			if (!Node.IsValid()) return;
			const TArray<TSharedPtr<FGameplayTagNode>>& Children = Node->GetChildTagNodes();
			if (Children.Num() == 0)
			{
				OutLeafTags.Add(Node->GetCompleteTag());
			}
			else
			{
				for (const TSharedPtr<FGameplayTagNode>& Child : Children)
				{
					GatherLeafTags(Child);
				}
			}
		};

		for (const TSharedPtr<FGameplayTagNode>& Node : TagNodes)
		{
			GatherLeafTags(Node);
		}
	}

	bool TrySetGameplayTagOnHandle(TSharedPtr<IPropertyHandle> TagHandle, const FGameplayTag& Tag)
	{
		if (!TagHandle.IsValid()) return false;

		if (TagHandle->SetValueFromFormattedString(Tag.ToString()) == FPropertyAccess::Success)
		{
			return true;
		}
		void* DataPtr = nullptr;
		if (TagHandle->GetValueData(DataPtr) == FPropertyAccess::Success && DataPtr)
		{
			*static_cast<FGameplayTag*>(DataPtr) = Tag;
			return true;
		}
		return false;
	}

	void AddAllMissingLeafTags(TSharedPtr<IPropertyHandleArray> ArrayHandle,
		FName TagMemberPropertyName,
		const FString& RootPath,
		TSharedPtr<IPropertyUtilities> PropUtils)
	{
		if (!ArrayHandle.IsValid()) return;

		// Gather existing tags by iterating array elements
		TSet<FGameplayTag> ExistingTags;
		uint32 NumElems = 0;
		ArrayHandle->GetNumElements(NumElems);
		for (uint32 i = 0; i < NumElems; ++i)
		{
			if (TSharedPtr<IPropertyHandle> Elem = ArrayHandle->GetElement(i))
			{
				if (TSharedPtr<IPropertyHandle> TagProp = Elem->GetChildHandle(TagMemberPropertyName))
				{
					FString StrValue;
					if (TagProp->GetValueAsFormattedString(StrValue) == FPropertyAccess::Success)
					{
						if (!StrValue.IsEmpty())
						{
							ExistingTags.Add(FGameplayTag::RequestGameplayTag(FName(*StrValue), false));
						}
					}
				}
			}
		}

		// Gather all leaf tags under RootPath and add missing
		TArray<FGameplayTag> LeafTags;
		GatherLeafTagsUnder(RootPath, LeafTags);

		for (const FGameplayTag& Tag : LeafTags)
		{
			if (!ExistingTags.Contains(Tag))
			{
				ArrayHandle->AddItem();
				uint32 NewCount = 0;
				ArrayHandle->GetNumElements(NewCount);
				if (NewCount > 0)
				{
					if (TSharedPtr<IPropertyHandle> NewElem = ArrayHandle->GetElement(NewCount - 1))
					{
						if (TSharedPtr<IPropertyHandle> TagProp = NewElem->GetChildHandle(TagMemberPropertyName))
						{
							TrySetGameplayTagOnHandle(TagProp, Tag);
						}
					}
				}
			}
		}

		if (PropUtils.IsValid())
		{
			PropUtils->ForceRefresh();
		}
	}

	TSharedRef<SWidget> MakePropertyWidgetBox(TSharedPtr<IPropertyHandle> Handle,
		float MinDesiredWidth)
	{
		return SNew(SBox)
		.MinDesiredWidth(MinDesiredWidth)
		[
			Handle.IsValid() ? Handle->CreatePropertyValueWidget() : SNullWidget::NullWidget
		];
	}

	TSharedRef<SWidget> MakePropertyWidgetFixed(TSharedPtr<IPropertyHandle> Handle,
		float WidthOverride)
	{
		return SNew(SBox)
		.WidthOverride(WidthOverride)
		[
			Handle.IsValid() ? Handle->CreatePropertyValueWidget() : SNullWidget::NullWidget
		];
	}

	static bool IsPropertyHandleEmpty(TSharedPtr<IPropertyHandle> Handle)
	{
		if (!Handle.IsValid()) return true;
		const FProperty* Prop = Handle->GetProperty();
		if (Prop)
		{
			void* DataPtr = nullptr;
			if (Handle->GetValueData(DataPtr) == FPropertyAccess::Success && DataPtr)
			{
				if (Prop->IsA<FNameProperty>())
				{
					const FName* N = static_cast<FName*>(DataPtr);
					return !N || N->IsNone();
				}
				if (Prop->IsA<FTextProperty>())
				{
					const FText* T = static_cast<FText*>(DataPtr);
					if (!T) return true;
					FString S = T->ToString();
					S.TrimStartAndEndInline();
					return S.IsEmpty();
				}
			}
		}
		FString Display;
		if (Handle->GetValueAsDisplayString(Display) == FPropertyAccess::Success)
		{
			Display.TrimStartAndEndInline();
			Display.ReplaceInline(TEXT("\""), TEXT(""));
			if (Display.IsEmpty()) return true;
			if (Display.Equals(TEXT("None"), ESearchCase::IgnoreCase)) return true;
		}
		return false;
	}

	TSharedRef<SWidget> MakePropertyWidgetFixedWithValidationNote(
		TSharedPtr<IPropertyHandle> Handle,
		float WidthOverride,
		const FText& InvalidNoteText,
		FLinearColor InvalidColor)
	{
		return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(InvalidNoteText)
			.ColorAndOpacity(FSlateColor(InvalidColor))
			.Font(FAppStyle::Get().GetFontStyle("SmallFont"))
			.Visibility_Lambda([Handle]()
			{
				return IsPropertyHandleEmpty(Handle) ? EVisibility::Visible : EVisibility::Collapsed;
			})
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox)
			.WidthOverride(WidthOverride > 0.f ? WidthOverride : TOptional<float>())
			[
				Handle.IsValid() ? Handle->CreatePropertyValueWidget() : SNullWidget::NullWidget
			]
		]
		;
	}

	TSharedRef<SWidget> MakeEnumWithCustomBelow(
		TSharedPtr<IPropertyHandle> EnumHandle,
		TSharedPtr<IPropertyHandle> CustomHandle,
		float WidthOverride,
		const TCHAR* CustomEntryName,
		float CustomLeftIndent)
	{
		// Helper to test if the current enum value equals the Custom entry
		auto IsCustomSelected = [EnumHandle, CustomEntryName]() -> bool
		{
			if (!EnumHandle.IsValid()) return false;
			const FProperty* Prop = EnumHandle->GetProperty();
			if (!Prop) return false;

			UEnum* EnumDef = nullptr;
			if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(Prop))
			{
				EnumDef = EnumProp->GetEnum();
			}
			else if (const FByteProperty* ByteProp = CastField<FByteProperty>(Prop))
			{
				EnumDef = ByteProp->Enum;
			}

			// Try to read as an integer first
			int64 CurrentValue = 0;
			if (EnumHandle->GetValue(CurrentValue) == FPropertyAccess::Success && EnumDef)
			{
				// Compare by display name (preferred)
				const FText Disp = EnumDef->GetDisplayNameTextByValue(CurrentValue);
				if (Disp.ToString().Equals(CustomEntryName, ESearchCase::IgnoreCase))
				{
					return true;
				}
				// Fallback: compare by raw name string
				const FString NameStr = EnumDef->GetNameStringByValue(CurrentValue);
				if (NameStr.Equals(CustomEntryName, ESearchCase::IgnoreCase))
				{
					return true;
				}
				if (NameStr.EndsWith(FString("::") + CustomEntryName, ESearchCase::IgnoreCase))
				{
					return true;
				}
			}

			// Fallbacks: some enum handles don't support GetValue(int64). Compare display/formatted strings.
			{
				FString Display;
				if (EnumHandle->GetValueAsDisplayString(Display) == FPropertyAccess::Success)
				{
					Display.TrimStartAndEndInline();
					Display.ReplaceInline(TEXT("\""), TEXT(""));
					// Strip any qualifying scope (e.g., Enum::Custom)
					int32 ScopeIdx;
					if (Display.FindLastChar(':', ScopeIdx))
					{
						Display = Display.RightChop(ScopeIdx + 1);
					}
					if (Display.Equals(CustomEntryName, ESearchCase::IgnoreCase))
					{
						return true;
					}
				}
			}
			{
				FString Formatted;
				if (EnumHandle->GetValueAsFormattedString(Formatted) == FPropertyAccess::Success)
				{
					Formatted.TrimStartAndEndInline();
					Formatted.ReplaceInline(TEXT("\""), TEXT(""));
					int32 ScopeIdx;
					if (Formatted.FindLastChar(':', ScopeIdx))
					{
						Formatted = Formatted.RightChop(ScopeIdx + 1);
					}
					if (Formatted.Equals(CustomEntryName, ESearchCase::IgnoreCase))
					{
						return true;
					}
				}
			}
			return false;
		};

		// Helper to build a typed text box for the custom value (supports FName and FText)
		auto MakeCustomInput = [CustomHandle]() -> TSharedRef<SWidget>
		{
			if (!CustomHandle.IsValid()) return SNullWidget::NullWidget;
			const FProperty* Prop = CustomHandle->GetProperty();
			if (!Prop) return SNullWidget::NullWidget;

			if (Prop->IsA<FNameProperty>())
			{
				return SNew(SEditableTextBox)
				.HintText(FText::FromString(TEXT("Enter custom name")))
				.SelectAllTextWhenFocused(true)
				.Text_Lambda([CustomHandle]()
				{
					FName N = NAME_None; CustomHandle->GetValue(N);
					return FText::FromString(N.IsNone() ? TEXT("") : N.ToString());
				})
				.OnTextCommitted_Lambda([CustomHandle](const FText& NewText, ETextCommit::Type)
				{
					const FString S = NewText.ToString().TrimStartAndEnd();
					CustomHandle->SetValue(S.IsEmpty() ? NAME_None : FName(*S));
				});
			}
			if (Prop->IsA<FTextProperty>())
			{
				return SNew(SEditableTextBox)
				.HintText(FText::FromString(TEXT("Enter custom text")))
				.SelectAllTextWhenFocused(true)
				.Text_Lambda([CustomHandle]()
				{
					FText T; CustomHandle->GetValue(T);
					return T;
				})
				.OnTextCommitted_Lambda([CustomHandle](const FText& NewText, ETextCommit::Type)
				{
					CustomHandle->SetValue(NewText);
				});
			}
			// Fallback to property widget
			return CustomHandle->CreatePropertyValueWidget();
		};

		return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox)
			.WidthOverride(WidthOverride)
			[
				EnumHandle.IsValid() ? EnumHandle->CreatePropertyValueWidget() : SNullWidget::NullWidget
			]
		]
		+ SVerticalBox::Slot().AutoHeight()
		.Padding(FMargin(CustomLeftIndent, 2.f, 0.f, 0.f))
		[
			SNew(SBox)
			.WidthOverride(WidthOverride)
			[
				(CustomHandle.IsValid() ? MakeCustomInput() : SNullWidget::NullWidget)
			]
			.Visibility_Lambda([IsCustomSelected]()
			{
				return IsCustomSelected() ? EVisibility::Visible : EVisibility::Collapsed;
			})
		]
		;
	}

	TSharedRef<SWidget> MakeOverrideEnumPreview(
		TFunction<bool()> bShowOverride,
		TFunction<FText()> EnumDisplayText,
		TFunction<bool()> bShowCustomBelow,
		TFunction<FText()> CustomDisplayText,
		float WidthOverride,
		float CustomLeftIndent)
	{
		return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([bShowOverride]()
			{
				return bShowOverride() ? EVisibility::Visible : EVisibility::Collapsed;
			})
			// Label
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Override:")))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f,0.7f,0.7f)))
				.Font(FAppStyle::Get().GetFontStyle("SmallFont"))
			]
			// Disabled enum-like chooser
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0,2,0,0))
			[
				SNew(SBox)
				.WidthOverride(WidthOverride)
				[
					SNew(SComboButton)
					.IsEnabled(false)
					.ButtonContent()
					[
						SNew(STextBlock)
						.Text_Lambda([EnumDisplayText]() { return EnumDisplayText(); })
					]
				]
			]
			// Disabled custom input when Custom override is selected
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(CustomLeftIndent,2,0,0))
			[
				SNew(SBox)
				.WidthOverride(WidthOverride)
				[
					SNew(SEditableTextBox)
					.IsEnabled(false)
					.Text_Lambda([CustomDisplayText]() { return CustomDisplayText(); })
				]
				.Visibility_Lambda([bShowCustomBelow]()
				{
					return bShowCustomBelow() ? EVisibility::Visible : EVisibility::Collapsed;
				})
			]
		]
		;
	}
}

#endif // WITH_EDITOR
