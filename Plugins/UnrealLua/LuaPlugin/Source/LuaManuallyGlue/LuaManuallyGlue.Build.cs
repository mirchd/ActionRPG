// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
 
public class LuaManuallyGlue : ModuleRules
{          
	private string LuaLibPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Lua/")); }
    }
	public LuaManuallyGlue(ReadOnlyTargetRules Target) : base(Target)
	{    
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs; 
        //bFasterWithoutUnity = true; 
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory,"Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory,"Private"));
            
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "HeadMountedDisplay",
            "AIModule",
            "RHI",
            "RenderCore",
            "Foliage",
            "GameplayTasks",
            "HTTP",
            "Slate",
            "SlateCore",
            "UMG",
            "AssetRegistry",
            "Networking",
            "Sockets",
            "Lua53",
            "LuaPluginRuntime"
        });
    }
}
