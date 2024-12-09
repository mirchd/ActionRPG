# Title
ActionRPG (not just ARPG) for ue5.

# Cmd

### generate sln
- "D:\program\Unreal\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe" /projectfiles "D:\ActionRPG\ActionRPG.uproject"<br>
- C:\WINDOWS\system32\cmd.exe /c ""D:/UnrealEngine/Engine/Build/BatchFiles/Build.bat"  -projectfiles -project="D:/ActionRPG/ActionRPG.uproject" -game -engine -progress -log="D:\ActionRPG/Saved/Logs/UnrealVersionSelector-2024.11.17-08.15.46.log""<br>
- dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -projectfiles -project="D:/ActionRPG/ActionRPG.uproject" -game -engine -progress -log="D:\ActionRPG/Saved/Logs/UnrealVersionSelector-2024.11.17-08.15.46.log"<br>


**Running UnrealBuildTool**<br>
- dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="UnrealEditor Win64 Debug" -Target="ShaderCompileWorker Win64 Development -Quiet" -WaitMutex -FromMsBuild<br>
- dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="ActionRPGEditor Win64 Debug -Project=\"D:\ActionRPG\ActionRPG.uproject\"" -Target="ShaderCompileWorker Win64 Development -Quiet" -WaitMutex -FromMsBuild<br>

**Running Internal UnrealHeaderTool**<br>
"D:\UnrealEngine\Engine\Binaries\Win64\UnrealHeaderTool.exe" "D:\ActionRPG\ActionRPG.uproject" "D:\ActionRPG\Intermediate\Build\Win64\ActionRPGEditor\Debug\ActionRPGEditor.uhtmanifest" -LogCmds="loginit warning, logexit warning, logdatabase error" -Unattended -WarningsAsErrors -abslog="D:\UnrealEngine\Engine\Programs\UnrealBuildTool\Log_UHT.txt"<br>

**start**<br>
"D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug.exe" "D:\ActionRPG\ActionRPG.uproject" -skipcompile<br>
**attach**<br>
-waitforattach/-WaitForDebugger<br>

**Refresh platform status**<br>
"D:/UnrealEngine/Engine/Build/BatchFiles/RunUAT.bat"  -ScriptsForProject="D:/ActionRPG/ActionRPG.uproject" Turnkey -utf8output -WaitForUATMutex -command=VerifySdk -ReportFilename="D:/ActionRPG/Intermediate/TurnkeyReport_4.log" -log="D:/ActionRPG/Intermediate/TurnkeyLog_4.log" -project="D:/ActionRPG/ActionRPG.uproject"  -platform=all<br>

**start engine**<br>
"D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug.exe"<br>

**pack Android**<br>
- "D:/UnrealEngine/Engine/Build/BatchFiles/RunUAT.bat"  -ScriptsForProject="D:/ActionRPG/ActionRPG.uproject" Turnkey -command=VerifySdk -platform=Android -UpdateIfNeeded -EditorIO -EditorIOPort=54770  -project="D:/ActionRPG/ActionRPG.uproject" BuildCookRun -nop4 -utf8output -nocompileeditor -skipbuildeditor -cook  -project="D:/ActionRPG/ActionRPG.uproject" -target=ActionRPG  -unrealexe="D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug-Cmd.exe" -platform=Android  -cookflavor=ASTC -SkipCookingEditorContent -stage -archive -package -build -pak -iostore -compressed -archivedirectory="D:/ActionRPG/Build/apk" -manifests -CrashReporter -clientconfig=Shipping -nocompile -nocompileuat<br>
- dotnet  AutomationTool.dll -ScriptsForProject="D:/ActionRPG/ActionRPG.uproject" Turnkey -command=VerifySdk -platform=Android -UpdateIfNeeded -EditorIO -EditorIOPort=54770  -project="D:/ActionRPG/ActionRPG.uproject" BuildCookRun -nop4 -utf8output -nocompileeditor -skipbuildeditor -cook  -project="D:/ActionRPG/ActionRPG.uproject" -target=ActionRPG  -unrealexe="D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug-Cmd.exe" -platform=Android  -cookflavor=ASTC -SkipCookingEditorContent -stage -archive -package -build -pak -iostore -compressed -archivedirectory="D:/ActionRPG/Build/apk" -manifests -CrashReporter -clientconfig=Shipping -nocompile -nocompileuat<br>
- "D:\UnrealEngine\Engine\Binaries\ThirdParty\DotNet\6.0.302\windows\dotnet.exe" "D:\UnrealEngine\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="UnrealPak Win64 Development -Project=D:\ActionRPG\ActionRPG.uproject -Manifest=D:\UnrealEngine\Engine\Intermediate\Build\Manifest-1-UnrealPak-Win64-Development.xml" -Target="ActionRPG Android Shipping -Project=D:\ActionRPG\ActionRPG.uproject -Manifest=D:\UnrealEngine\Engine\Intermediate\Build\Manifest-2-ActionRPG-Android-Shipping.xml  -remoteini=\"D:\ActionRPG\"  -skipdeploy " -log="D:\UnrealEngine\Engine\Programs\AutomationTool\Saved\Logs\UBA-UnrealPak-Win64-Development.txt"<br>
- "D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug-Cmd.exe" "D:\ActionRPG\ActionRPG.uproject" -run=Cook  -TargetPlatform=Android_ASTC  -unversioned -skipeditorcontent -fileopenlog -manifests -abslog="D:\UnrealEngine\Engine\Programs\AutomationTool\Saved\Cook-2024.11.12-15.02.24.txt" -stdout -CrashForUAT -unattended -NoLogTimes  -UTF8Output<br>


**cook**<br>
"D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug-Cmd.exe" "D:\ActionRPG\ActionRPG.uproject" -run=Cook  -TargetPlatform=Android_ASTC  -unversioned -skipeditorcontent -fileopenlog -manifests -abslog="D:\UnrealEngine\Engine\Programs\AutomationTool\Saved\Cook-2024.12.09-11.10.43.txt" -stdout -CrashForUAT -unattended -NoLogTimes  -UTF8Output<br>


# Special Thanks
- [RealtimeMeshComponent](https://github.com/TriAxis-Games/RealtimeMeshComponent)
- [LuaMachine](https://github.com/rdeioris/LuaMachine)
- [ActionRPG](https://docs.unrealengine.com/4.27/zh-CN/Resources/SampleGames/ARPG/)
- [UE5.5](https://www.unrealengine.com/zh-CN/unreal-engine-5)
- [vc-ue-extensions](https://github.com/microsoft/vc-ue-extensions)
- [StreetMap](https://github.com/ue4plugins/StreetMap)
- [HotPatcher](https://github.com/hxhb/HotPatcher)
- [PlatformUtils](https://github.com/hxhb/PlatformUtils)
- [HoudiniEngineForUnreal](https://github.com/sideeffects/HoudiniEngineForUnreal)


