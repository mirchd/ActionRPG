# Title
ActionRPG (not just ARPG) for ue5.

# Cmd

### generate sln
1."D:\program\Unreal\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe" /projectfiles "D:\ActionRPG\ActionRPG.uproject"<br>
1.1.C:\WINDOWS\system32\cmd.exe /c ""D:/UnrealEngine/Engine/Build/BatchFiles/Build.bat"  -projectfiles -project="D:/ActionRPG/ActionRPG.uproject" -game -engine -progress -log="D:\ActionRPG/Saved/Logs/UnrealVersionSelector-2024.12.18-12.56.37.log""<br>
1.1.1.dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -projectfiles -project="D:/ActionRPG/ActionRPG.uproject" -game -engine -progress -log="D:\ActionRPG/Saved/Logs/UnrealVersionSelector-2024.12.18-12.56.37.log"<br>


**Running UnrealBuildTool**<br>
- dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="UnrealEditor Win64 Debug" -Target="ShaderCompileWorker Win64 Development -Quiet" -WaitMutex -FromMsBuild -architecture=x64<br>
- dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="ActionRPGEditor Win64 Debug -Project=\"D:\ActionRPG\ActionRPG.uproject\"" -Target="ShaderCompileWorker Win64 Development -Project=\"D:\ActionRPG\ActionRPG.uproject\" -Quiet" -WaitMutex -FromMsBuild -architecture=x64<br>

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
1."cmd.exe"  /c ""D:/UnrealEngine/Engine/Build/BatchFiles/RunUAT.bat"  -ScriptsForProject="D:/ActionRPG/ActionRPG.uproject" Turnkey -command=VerifySdk -platform=Android -UpdateIfNeeded -EditorIO -EditorIOPort=63997  -project="D:/ActionRPG/ActionRPG.uproject" BuildCookRun -nop4 -utf8output -nocompileeditor -skipbuildeditor -cook  -project="D:/ActionRPG/ActionRPG.uproject" -target=ActionRPG  -unrealexe="D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug-Cmd.exe" -platform=Android  -cookflavor=ASTC -SkipCookingEditorContent -stage -archive -package -build -pak -iostore -compressed -archivedirectory="D:/ActionRPG/Build" -manifests -CrashReporter -clientconfig=Debug" -nocompile -nocompileuat<br>
Current directory: D:\UnrealEngine\Engine\Binaries\DotNET\AutomationTool\<br>

1.1.dotnet  AutomationTool.dll -ScriptsForProject="D:/ActionRPG/ActionRPG.uproject" Turnkey -command=VerifySdk -platform=Android -UpdateIfNeeded -EditorIO -EditorIOPort=63997  -project="D:/ActionRPG/ActionRPG.uproject" BuildCookRun -nop4 -utf8output -nocompileeditor -skipbuildeditor -cook  -project="D:/ActionRPG/ActionRPG.uproject" -target=ActionRPG  -unrealexe="D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug-Cmd.exe" -platform=Android  -cookflavor=ASTC -SkipCookingEditorContent -stage -archive -package -build -pak -iostore -compressed -archivedirectory="D:/ActionRPG/Build" -manifests -CrashReporter -clientconfig=Debug -nocompile -nocompileuat<br>
Current directory: D:\UnrealEngine\<br>

1.1.1."D:\UnrealEngine\Engine\Binaries\ThirdParty\DotNet\8.0.300\win-x64\dotnet.exe" "D:\UnrealEngine\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="UnrealPak Win64 Development -Project=D:\ActionRPG\ActionRPG.uproject -Manifest=D:\UnrealEngine\Engine\Intermediate\Build\Manifest-1-UnrealPak-Win64-Development.xml" -Target="ActionRPG Android Debug -Project=D:\ActionRPG\ActionRPG.uproject -Manifest=D:\UnrealEngine\Engine\Intermediate\Build\Manifest-2-ActionRPG-Android-Debug.xml  -remoteini=\"D:\ActionRPG\"  -skipdeploy " -log="D:\UnrealEngine\Engine\Programs\AutomationTool\Saved\Logs\UBA-UnrealPak-Win64-Development.txt"<br>
Current directory: D:\UnrealEngine\Engine\Source\<br>

1.2."D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug-Cmd.exe" "D:\ActionRPG\ActionRPG.uproject" -run=Cook  -TargetPlatform=Android_ASTC  -unversioned -skipeditorcontent -fileopenlog -manifests -abslog="D:\UnrealEngine\Engine\Programs\AutomationTool\Saved\Cook-2025.03.06-17.39.37.txt" -stdout -CrashForUAT -unattended -NoLogTimes  -UTF8Output<br>
Current directory: D:\UnrealEngine\Engine\Binaries\Win64\<br>

1.3. dotnet  AutomationTool.dll -ScriptsForProject="D:/ActionRPG/ActionRPG.uproject" Turnkey -command=VerifySdk -platform=Android -UpdateIfNeeded -EditorIO -EditorIOPort=63997  -project="D:/ActionRPG/ActionRPG.uproject" BuildCookRun -nop4 -utf8output -nocompileeditor -skipbuildeditor -cook  -project="D:/ActionRPG/ActionRPG.uproject" -target=ActionRPG  -unrealexe="D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug-Cmd.exe" -platform=Android  -cookflavor=ASTC -SkipCookingEditorContent -stage -archive -package -build -pak -iostore -compressed -archivedirectory="D:/ActionRPG/Build" -manifests -CrashReporter -clientconfig=Debug -nocompile -nocompileuat<br>
Current directory: D:\UnrealEngine\<br>

1.4."D:\UnrealEngine\Engine\Binaries\Win64\UnrealPak.exe" "D:\ActionRPG\ActionRPG.uproject"  -cryptokeys="D:\ActionRPG\Saved\Cooked\Android_ASTC\ActionRPG\Metadata\Crypto.json" -patchpaddingalign=0 -compressionformats=Oodle -compresslevel=5 -compressmethod=Kraken  -platform=Android  -CreateMultiple="D:\UnrealEngine\Engine\Programs\AutomationTool\Saved\Logs\PakCommands.txt"<br>
1.5."cmd.exe" /c "D:\ActionRPG\Intermediate\Android\arm64\gradle\rungradle.bat" :app:assembleDebug<br>


# Special Thanks
- [ActionRPG](https://docs.unrealengine.com/4.27/zh-CN/Resources/SampleGames/ARPG/)
- [UE5.6](https://www.unrealengine.com/zh-CN/unreal-engine-5)
- [HotPatcher](https://github.com/hxhb/HotPatcher)
- [PlatformUtils](https://github.com/hxhb/PlatformUtils)
- [LuaMachine](https://github.com/rdeioris/LuaMachine)
- [StreetMap](https://github.com/ue4plugins/StreetMap)
- [HoudiniEngineForUnreal](https://github.com/sideeffects/HoudiniEngineForUnreal)


