// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class GP4Prototype : ModuleRules
{
	public GP4Prototype(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.AddRange(new[] { "GP4Prototype/Public" });
		PrivateIncludePaths.AddRange(new[] { "GP4Prototype/Private" });
	
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"GameplayTags", 
			"AIModule", 
			"DeveloperSettings",
			"UMG",
			"Slate",
			"SlateCore",
            "NavigationSystem",
            "SimplifiedDebugMessage",
            "Niagara",
            "MetasoundEngine"
        });
		
		PrivateDependencyModuleNames.AddRange(new string[] { 
            "MetasoundFrontend",
            "MetasoundGraphCore",
            "AudioExtensions"
        });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
