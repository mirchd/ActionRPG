// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class LuaPluginEditor : ModuleRules
{
	public LuaPluginEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory,"Private"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory,"Public"));
        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
                "UnrealEd", // for FAssetEditorManager
				"LuaPluginRuntime",
                "Projects",
                "InputCore",
                "LevelEditor",
                "Engine",
                "Slate",
                "SlateCore",
                "BlueprintGraph",
            }
			);
	}
}
