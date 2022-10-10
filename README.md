# Title
ActionRPG (not just ARPG) for ue5.

# Cmd

**generate sln**
"D:/program/Unreal/UnrealEngine/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe"  -projectfiles -project="D:/program/Unreal/ActionRPG/ActionRPG.uproject" -game -engine -progress -log="D:\program\Unreal\ActionRPG/Saved/Logs/UnrealVersionSelector-2022.09.28-19.13.09.log"

**parsing headers for ActionRPGEditor
dotnet  "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -Target="ActionRPGEditor Win64 Debug -Project=\"D:\program\Unreal\ActionRPG\ActionRPG.uproject\"" -Target="ShaderCompileWorker Win64 Development -Quiet" -WaitMutex -FromMsBuild
"D:\program\Unreal\UnrealEngine\Engine\Binaries\Win64\UnrealHeaderTool.exe" "D:\program\Unreal\ActionRPG\ActionRPG.uproject" "D:\program\Unreal\ActionRPG\Intermediate\Build\Win64\ActionRPGEditor\Debug\ActionRPGEditor.uhtmanifest" -LogCmds="loginit warning, logexit warning, logdatabase error" -Unattended -WarningsAsErrors -abslog="D:\program\Unreal\UnrealEngine\Engine\Programs\UnrealBuildTool\Log_UHT.txt"

# Special Thanks
- [HotPatcher](https://github.com/hxhb/HotPatcher.git)
- [unreal.lua](https://github.com/asqbtcupid/unreal.lua.git)
- [UE5]
