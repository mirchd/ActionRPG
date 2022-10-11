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
rd /s/q %~dp0Plugins\UnLua\Binaries
rd /s/q %~dp0Plugins\UnLua\Intermediate
rd /s/q %~dp0Plugins\UnLuaExtensions\Binaries
rd /s/q %~dp0Plugins\UnLuaExtensions\Intermediate
rd /s/q %~dp0Plugins\UnLuaTestSuite\Binaries
rd /s/q %~dp0Plugins\UnLuaTestSuite\Intermediate
del %~dp0*.sln

FOR /F "tokens=*" %%G IN ('DIR /B /AD /S bin') DO RMDIR /S /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /AD /S obj') DO RMDIR /S /Q "%%G"

pause