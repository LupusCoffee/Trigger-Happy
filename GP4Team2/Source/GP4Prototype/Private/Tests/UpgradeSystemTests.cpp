// UpgradeSystemTests.cpp - Automation tests for the attribute & upgrade systems

#include "Misc/AutomationTest.h"
#include "Systems/AttributeSystem/AttributeComponent.h"
#include "Systems/UpgradeSystem/RarityData.h"
#include "Systems/UpgradeSystem/UpgradeCardData.h"
#include "Systems/UpgradeSystem/UpgradeManagerComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttributeRecalculationTest, "GP4.Attribute.Recalculate.AddMulOverride", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAttributeRecalculationTest::RunTest(const FString& Parameters)
{
	UAttributeComponent* Comp = NewObject<UAttributeComponent>(GetTransientPackage());

	// Register and use a test tag
	FAttribute& Attr = Comp->RegisterAndGetAttribute(TEXT("Attribute.Test"), TEXT("Test attribute"));
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Attribute.Test")));
	Comp->SetAttributeBaseValue(Tag, 10.f);

	// Addition +5 and Multiplication x2 => (10 + 5) * 2 = 30
	FModifier Add; Add.Type = EModificationType::Addition; Add.Value = 5.f;
	FModifier Mul; Mul.Type = EModificationType::Multiplication; Mul.Value = 2.f;
	Comp->AddModifier(Tag, Add);
	Comp->AddModifier(Tag, Mul);
	TestEqual(TEXT("Add+Mul recalculation"), Comp->GetAttributeValue(Tag), 30.f);

	// Override 7 => 7
	FModifier Ov; Ov.Type = EModificationType::Override; Ov.Value = 7.f;
	Comp->AddModifier(Tag, Ov);
	TestEqual(TEXT("Override sets value"), Comp->GetAttributeValue(Tag), 7.f);

	// Remove override -> back to 30
	Comp->RemoveModifier(Tag, Ov);
	TestEqual(TEXT("Removing override restores computed value"), Comp->GetAttributeValue(Tag), 30.f);

	// Clear -> back to base 10
	Comp->ClearModifiers(Tag);
	TestEqual(TEXT("Clearing modifiers restores base"), Comp->GetAttributeValue(Tag), 10.f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRarityRollZeroWeightTest, "GP4.Upgrade.RollRarity.ZeroWeightFallbackCommon", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRarityRollZeroWeightTest::RunTest(const FString& Parameters)
{
	UUpgradeManagerComponent* Manager = NewObject<UUpgradeManagerComponent>(GetTransientPackage());
	// Leave RarityDataList empty to force total=0 path
	const ERarity Rolled = Manager->RollRarity(/*Difficulty*/0);
	TestEqual(TEXT("RollRarity falls back to Common when total weight is 0"), Rolled, ERarity::Common);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRollUpgradesEmptyPoolByFloorTest, "GP4.Upgrade.RollUpgrades.EmptyPoolByFloorGating", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRollUpgradesEmptyPoolByFloorTest::RunTest(const FString& Parameters)
{
	UUpgradeManagerComponent* Manager = NewObject<UUpgradeManagerComponent>(GetTransientPackage());

	// Build a card that's only valid for floors >= 1
	UUpgradeCardData* Card = NewObject<UUpgradeCardData>(GetTransientPackage());
	Card->DisplayName = FText::FromString(TEXT("Test Card"));
	Card->Rarity = ERarity::Common;
	Card->Rules.MinFloor = 1; // current floor from manager will be -1 (no LevelManager), so invalid

	// Give it one harmless modifier so application is possible if it ever passed validation
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Attribute.TestRoll")));
	{
		FAttributeModifierEntry E;
		E.TargetAttribute = Tag;
		E.Type = EModificationType::Addition;
		E.Value = 1.f;
		Card->Modifiers.Add(E);
	}

	Manager->AllUpgradeCards = { Card };

	UAttributeComponent* Comp = NewObject<UAttributeComponent>(GetTransientPackage());
	Comp->RegisterAndGetAttribute(TEXT("Attribute.TestRoll"), TEXT("Test tag for roll"));

	TArray<UUpgradeCardData*> Rolled = Manager->RollUpgrades(Comp, /*NumCards*/3);
	TestTrue(TEXT("No cards should be rolled when all candidates are gated by floor"), Rolled.Num() == 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickRandomModifierEmptyTest, "GP4.Upgrade.PickRandomModifier.EmptyInputSafe", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPickRandomModifierEmptyTest::RunTest(const FString& Parameters)
{
	UUpgradeManagerComponent* Manager = NewObject<UUpgradeManagerComponent>(GetTransientPackage());
	TArray<FModifier> Empty;
	FModifier Picked = Manager->PickRandomModifier(Empty);
	TestTrue(TEXT("Returned modifier ID should be invalid for empty input"), !Picked.ModifierID.IsValid());
	TestEqual(TEXT("Returned modifier value should be default 0"), Picked.Value, 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttributeIntegerTypeRoundingTest, "GP4.Attribute.NumericType.IntegerRounding", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAttributeIntegerTypeRoundingTest::RunTest(const FString& Parameters)
{
	UAttributeComponent* Comp = NewObject<UAttributeComponent>(GetTransientPackage());
	FAttribute& Attr = Comp->RegisterAndGetAttribute(TEXT("Attribute.IntTest"), TEXT("Int attribute test"));
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Attribute.IntTest")));
	Comp->SetAttributeBaseValue(Tag, 10.0f);
	Comp->SetAttributeNumericType(Tag, EAttributeNumericType::Integer);
	Comp->SetAttributeRoundingMode(Tag, EAttributeRoundingMode::RoundToNearest);

	// Add +0.6 -> 10.6 -> rounds to 11
	FModifier Add; Add.Type = EModificationType::Addition; Add.Value = 0.6f;
	Comp->AddModifier(Tag, Add);
	TestEqual(TEXT("Integer type rounds to nearest"), Comp->GetAttributeValueInt(Tag), 11);

	// Override 7.4 -> rounds to 7
	FModifier Ov; Ov.Type = EModificationType::Override; Ov.Value = 7.4f;
	Comp->AddModifier(Tag, Ov);
	TestEqual(TEXT("Override then round"), Comp->GetAttributeValueInt(Tag), 7);

	return true;
}
