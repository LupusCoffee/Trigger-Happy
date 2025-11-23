using UnrealBuildTool;

public class GP4PrototypeEditor : ModuleRules
{
    public GP4PrototypeEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(
                [
                    "Core"
                ]
            );

            PrivateDependencyModuleNames.AddRange(
                [
                    "CoreUObject",
                    "Engine",
                    "UnrealEd",
                    "PropertyEditor",
                    "Slate",
                    "SlateCore",
                    "InputCore",
                    "GameplayTags",
                    "GameplayTagsEditor",
                    "AssetRegistry",
                    "ToolMenus",
                    "AppFramework",
                    "ContentBrowser",
                    "ContentBrowserData",
                    "Kismet",
                    "KismetCompiler",
                    "GP4Prototype"
                ]
            );
        }
    }
}