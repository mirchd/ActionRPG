# Title
ActionRPG (not just ARPG) for ue5.

# Cmd

### generate sln
1.  "D:\program\Unreal\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe" /projectfiles "D:\ActionRPG\ActionRPG.uproject"
2.  "D:/UnrealEngine/Engine/Build/BatchFiles/Build.bat"  -projectfiles -project="D:/ActionRPG/ActionRPG.uproject" -game -engine -progress -log="D:/ActionRPG/Saved/Logs/UnrealVersionSelector-2024.03.08-08.48.16.log"
3.  dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -projectfiles -project="D:/ActionRPG/ActionRPG.uproject" -game -engine -progress -log="D:/ActionRPG/Saved/Logs/UnrealVersionSelector-2024.03.08-08.56.50.log"


**Running UnrealBuildTool**
dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="UnrealEditor Win64 Debug" -Target="ShaderCompileWorker Win64 Development -Quiet" -WaitMutex -FromMsBuild
dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="ActionRPGEditor Win64 Debug -Project=\"D:\ActionRPG\ActionRPG.uproject\"" -Target="ShaderCompileWorker Win64 Development -Quiet" -WaitMutex -FromMsBuild

**Running Internal UnrealHeaderTool**
"D:\UnrealEngine\Engine\Binaries\Win64\UnrealHeaderTool.exe" "D:\ActionRPG\ActionRPG.uproject" "D:\ActionRPG\Intermediate\Build\Win64\ActionRPGEditor\Debug\ActionRPGEditor.uhtmanifest" -LogCmds="loginit warning, logexit warning, logdatabase error" -Unattended -WarningsAsErrors -abslog="D:\UnrealEngine\Engine\Programs\UnrealBuildTool\Log_UHT.txt"

**start**
"D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug.exe" "D:\ActionRPG\ActionRPG.uproject" -skipcompile

# Special Thanks
- [RealtimeMeshComponent](https://github.com/TriAxis-Games/RealtimeMeshComponent.git)
- [UnLua](https://github.com/Tencent/UnLua.git)
- [ActionRPG](https://docs.unrealengine.com/4.27/zh-CN/Resources/SampleGames/ARPG/)
- [UE5](https://www.unrealengine.com/zh-CN/unreal-engine-5)
- [vc-ue-extensions](https://github.com/microsoft/vc-ue-extensions.git)
- [StreetMap](https://github.com/ue4plugins/StreetMap)


