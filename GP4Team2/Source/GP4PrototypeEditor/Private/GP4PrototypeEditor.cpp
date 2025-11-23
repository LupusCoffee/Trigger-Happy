#include "GP4PrototypeEditor.h"
#include "ContentBrowserMenuContexts.h"
#include "GP4PrototypeEditor/Public/Customizations/AgentAttributeCustomization.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Systems/UpgradeSystem/UpgradeManagerComponent.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "ToolMenus.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "Customizations/AttributeModifierCustomization.h"
#include "Engine/World.h"
#include "Systems/UpgradeSystem/UpgradeCardData.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Engine/Blueprint.h"
#include "Customizations/SubtitleNodeCustomization.h"
#include "Customizations/SubtitleScriptDataCustomization.h"

#define LOCTEXT_NAMESPACE "FGP4PrototypeEditorModule"

void FGP4PrototypeEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FModuleManager::Get().LoadModuleChecked("GameplayTagsEditor");

	PropertyModule.RegisterCustomPropertyTypeLayout(
		"AgentAttribute",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&AgentAttributeCustomization::MakeInstance)
	);

	PropertyModule.RegisterCustomPropertyTypeLayout("AttributeModifierEntry",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&AttributeModifierCustomization::MakeInstance)
	);

	PropertyModule.RegisterCustomPropertyTypeLayout("SubtitleNode",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&SubtitleNodeCustomization::MakeInstance)
	);

	PropertyModule.RegisterCustomClassLayout(
		"SubtitleScriptData",
		FOnGetDetailCustomizationInstance::CreateStatic(&FSubtitleScriptDataCustomization::MakeInstance)
	);

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FGP4PrototypeEditorModule::ShutdownModule()
{
	// No explicit cleanup needed.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGP4PrototypeEditorModule, GP4PrototypeEditor)