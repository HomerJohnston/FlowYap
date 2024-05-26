﻿using UnrealBuildTool;

public class FlowYapEditor : ModuleRules
{
    public FlowYapEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                
                "UnrealEd",
                "PropertyEditor",
                "AssetTools",
                "DetailCustomizations",
                
                "Flow",
                "FlowEditor",
                "FlowYap", 
                
                "KismetWidgets", 
                
                "EditorStyle",
                "GraphEditor",
                
                "AkAudio",
                
                "EditorSubsystem",
            }
        );
    }
}