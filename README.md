# Title
ActionRPG (not just ARPG) for ue5.

# Cmd

**generate sln**
"D:/program/Unreal/UnrealEngine/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe"  -projectfiles -project="D:/program/Unreal/ActionRPG/ActionRPG.uproject" -game -engine -progress -log="D:\program\Unreal\ActionRPG/Saved/Logs/UnrealVersionSelector-2022.09.28-19.13.09.log"

**parsing headers for ActionRPGEditor**
dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="ActionRPGEditor Win64 Debug -Project=\"D:\program\Unreal\ActionRPG\ActionRPG.uproject\"" -Target="ShaderCompileWorker Win64 Development -Quiet" -WaitMutex -FromMsBuild
"D:\program\Unreal\UnrealEngine\Engine\Binaries\Win64\UnrealHeaderTool.exe" "D:\program\Unreal\ActionRPG\ActionRPG.uproject" "D:\program\Unreal\ActionRPG\Intermediate\Build\Win64\ActionRPGEditor\Debug\ActionRPGEditor.uhtmanifest" -LogCmds="loginit warning, logexit warning, logdatabase error" -Unattended -WarningsAsErrors -abslog="D:\program\Unreal\UnrealEngine\Engine\Programs\UnrealBuildTool\Log_UHT.txt"

**start**
"D:\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Win64-Debug.exe" "D:\ActionRPG\ActionRPG.uproject" -skipcompile

# Special Thanks
- [RealtimeMeshComponent](https://github.com/TriAxis-Games/RealtimeMeshComponent.git)
- [UnLua](https://github.com/Tencent/UnLua.git)
- [ActionRPG](https://docs.unrealengine.com/4.27/zh-CN/Resources/SampleGames/ARPG/)
- [UE5](https://www.unrealengine.com/zh-CN/unreal-engine-5)
- [vc-ue-extensions](https://github.com/microsoft/vc-ue-extensions.git)
- [StreetMap](https://github.com/ue4plugins/StreetMap)


