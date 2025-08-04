// Copyright 2018-2025 - Roberto De Ioris

using UnrealBuildTool;

public class LuaMachine : ModuleRules
{
    protected enum LuaVMType
    {
        Lua53,
        Lua54,
        Luau,
        LuaJIT,
        Unknown
    }

    protected LuaVMType VMType = LuaVMType.Lua53;

    public LuaMachine(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

        PublicIncludePaths.AddRange(
            new string[]
            {
				// ... add public include paths required here ...
			}
        );


        PrivateIncludePaths.AddRange(
            new string[]
            {
				// ... add other private include paths required here ...
			}
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "HTTP",
                "Json",
                "PakFile"
				// ... add other public dependencies that you statically link with here ...
			}
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                "InputCore",
				// ... add private dependencies that you statically link with here ...	
			}
        );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
        );

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "Projects"
            });
        }

        string ThirdPartyDirectory = System.IO.Path.Combine(ModuleDirectory, "..", "ThirdParty");

        if (VMType == LuaVMType.Lua53)
        {
            PublicDefinitions.Add("LUAMACHINE_LUA53=1");

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "x64", "liblua53_win64.lib"));
            }

            else if (Target.Platform == UnrealTargetPlatform.Mac)
            {
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "x64", "liblua53_mac.a"));
            }

            else if (Target.Platform == UnrealTargetPlatform.Linux)
            {
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "x64", "liblua53_linux64.a"));
            }

            else if (Target.Platform == UnrealTargetPlatform.LinuxArm64)
            {
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "ARM64", "liblua53_linux_aarch64.a"));
            }

            else if (Target.Platform == UnrealTargetPlatform.Android)
            {
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "ARMv7", "liblua53_android.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "ARM64", "liblua53_android64.a"));
				// build android x86_64
				PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "x86_64", "liblua53_android32_64.a"));
            }

            else if (Target.Platform == UnrealTargetPlatform.IOS)
            {
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "ARM64", "liblua53_ios.a"));
            }
        }
        else
        {
            PublicDefinitions.Add("LUAMACHINE_LUA53=0");
        }

        if (VMType == LuaVMType.Luau)
        {
            PublicDefinitions.Add("LUAMACHINE_LUAU=1");
            PrivateIncludePaths.Add(System.IO.Path.Combine(ThirdPartyDirectory, "luau"));
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "Luau.Ast_win64.lib"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "Luau.Compiler_win64.lib"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "Luau.VM_win64.lib"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "Luau.Config_win64.lib"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "Luau.Analysis_win64.lib"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "Luau.EqSat_win64.lib"));
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac)
            {
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Ast_mac.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Compiler_mac.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.VM_mac.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Config_mac.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Analysis_mac.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.EqSat_mac.a"));
            }
            else if (Target.Platform == UnrealTargetPlatform.IOS)
            {
                bEnableExceptions = true;
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Ast_ios.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Compiler_ios.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.VM_ios.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Config_ios.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Analysis_ios.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.EqSat_ios.a"));
            }
            else if (Target.Platform == UnrealTargetPlatform.Android)
            {
                bEnableExceptions = true;
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Ast_android64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Compiler_android64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.VM_android64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Config_android64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Analysis_android64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.EqSat_android64.a"));

                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Ast_android32.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Compiler_android32.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.VM_android32.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Config_android32.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Analysis_android32.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.EqSat_android32.a"));
            }
            else if (Target.Platform == UnrealTargetPlatform.Linux)
            {
                bEnableExceptions = true;
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Ast_linux_x64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Compiler_linux_x64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.VM_linux_x64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Config_linux_x64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.EqSat_linux_x64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Analysis_linux_x64.a"));
            }
            else if (Target.Platform == UnrealTargetPlatform.LinuxArm64)
            {
                bEnableExceptions = true;
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Ast_linux_arm64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Compiler_linux_arm64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.VM_linux_arm64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Config_linux_arm64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.Analysis_linux_arm64.a"));
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "lib", "libLuau.EqSat_linux_arm64.a"));
            }
        }
        else
        {
            PublicDefinitions.Add("LUAMACHINE_LUAU=0");
        }

        if (VMType == LuaVMType.LuaJIT)
        {
            PublicDefinitions.Add("LUAMACHINE_LUAJIT=1");
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PublicAdditionalLibraries.Add(System.IO.Path.Combine(ThirdPartyDirectory, "x64", "luajit_win64.lib"));
            }
        }
        else
        {
            PublicDefinitions.Add("LUAMACHINE_LUAJIT=0");
        }
    }
}
