// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PlatformUtils : ModuleRules
{
	public PlatformUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
               // ... add public include paths required here ...
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Core",
                "Engine",
                "ApplicationCore"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				
				
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        string ThirdPartyPath = Path.Combine(ModuleDirectory, "ThirdParty");

		if(Target.Platform == UnrealTargetPlatform.IOS)
        {
            PublicAdditionalFrameworks.Add(
                    new Framework(
                        "Reachability",
                        "ThirdParty/IOS/Reachability.embeddedframework.zip"
                    )
                );

            PublicAdditionalFrameworks.Add(
                new Framework(
                    "SSKeychain",
                    "ThirdParty/IOS/SSKeychain.embeddedframework.zip"// ,
                    // "SSKeychain.framework/SSKeychain.bundle"
                )
            );

            PublicFrameworks.AddRange(
                new string[]
                {
                        "SystemConfiguration",
                        "Security"
                }
            );

            PublicAdditionalLibraries.Add("z");
            PublicAdditionalLibraries.Add("sqlite3");
        }

        if(Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
            AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", Path.Combine(ThirdPartyPath, "Android/PlatformUtils_UPL_Android.xml")));
        }
	}
}
