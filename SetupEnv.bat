:: configure the build environment

@if not defined _echo echo off
for /f "usebackq delims=" %%i in (` "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -prerelease -latest -property installationPath`) do (
  if exist "%%i\Common7\Tools\vsdevcmd.bat" (
    %comspec% /k "%%i\Common7\Tools\vsdevcmd.bat" -arch=amd64 -host_arch=amd64 %*
    exit /b
  )
)

rem Instance or command prompt not found
exit /b 2
:: Call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
:: add ourselves to the PATH 
