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
rd /s/q %~dp0Build\Android_ASTC
rd /s/q %~dp0Build\Apk
rd /s/q %~dp0Build\Android\src
del %~dp0Build\Android\project.properties
del %~dp0*.sln

FOR /F "tokens=*" %%G IN ('DIR /B /AD /S bin') DO RMDIR /S /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /AD /S obj') DO RMDIR /S /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /AD /S Binaries') DO RMDIR /S /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /AD /S Intermediate') DO RMDIR /S /Q "%%G"

pause