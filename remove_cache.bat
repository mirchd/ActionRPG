rd /s/q %~dp0.vs
rd /s/q %~dp0Android_ASTC
::rd /s/q %~dp0Build
rd /s/q %~dp0Intermediate
rd /s/q %~dp0Binaries
rd /s/q %~dp0Script
rd /s/q %~dp0PersistentDownloadDir
rd /s/q %~dp0Debug
rd /s/q %~dp0DerivedDataCache
rd /s/q %~dp0Saved
rd /s/q %~dp0Plugins\HotPatcher\Binaries
rd /s/q %~dp0Plugins\HotPatcher\Intermediate
rd /s/q %~dp0Plugins\LuaEditorPlugins\Binaries
rd /s/q %~dp0Plugins\LuaEditorPlugins\Intermediate
rd /s/q %~dp0Plugins\Lua53\Binaries
rd /s/q %~dp0Plugins\Lua53\Intermediate
rd /s/q %~dp0Plugins\UnrealLua\LuaDebugger\Binaries
rd /s/q %~dp0Plugins\UnrealLua\LuaDebugger\Intermediate
rd /s/q %~dp0Plugins\UnrealLua\LuaPlugin\Binaries
rd /s/q %~dp0Plugins\UnrealLua\LuaPlugin\Intermediate
rd /s/q %~dp0Plugins\UnrealLua\VarWatcher\Binaries
rd /s/q %~dp0Plugins\UnrealLua\VarWatcher\Intermediate
rd /s/q %~dp0Plugins\HorizonUIPlugin\Binaries
rd /s/q %~dp0Plugins\HorizonUIPlugin\Intermediate
rd /s/q %~dp0Plugins\WebSocket\Binaries
rd /s/q %~dp0Plugins\WebSocket\Intermediate
del %~dp0*.sln

FOR /F "tokens=*" %%G IN ('DIR /B /AD /S bin') DO RMDIR /S /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /AD /S obj') DO RMDIR /S /Q "%%G"

pause