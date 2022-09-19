@echo off
cls
setlocal EnableDelayedExpansion

 

:: Using the following external dependencies
::	https://github.com/fmtlib/fmt
::	https://github.com/ocornut/imgui/tree/docking
::	https://github.com/nlohmann/json

:: Documentation links
::	https://json.nlohmann.me/api/basic_json/
::	https://fmt.dev/latest/api.html
::	https://github.com/ocornut/imgui/wiki -- the imgui docs are kind of bad so you might just be better off looking in 
::                                           the imgui.h or googling your question

:: /EHsc	enables stdcpp exception handling behavior
:: /TP		treat all files as c++ regardless of file extension
:: /Wn		set compiler warning level
:: /GR-		disables run-time type information. We should never be using RTTI 
:: /Zi		Generates complete debugging information.
:: /c		compile without linking ( for building libs )
:: /utf-8	use the UTF8 execution charset 
:: /MP      Parallelize compilation of source files
SET "compiler_flags=/std:c++17 /W2 /MP /GR- /Zi /EHsc /utf-8  /D UNICODE /D _UNICODE /nologo /D NOMINMAX /D TRACY_ENABLE"
SET "link_to_libs=user32.lib dwmapi.lib ole32.lib kernel32.lib imgui.lib format.lib d3d11.lib dxgi.lib d3dcompiler.lib d2d1.lib dwrite.lib dxguid.lib TracyClient.lib" and 

:: available debug cfgs:
:: /D <flag>
:: DEBUG_WIN32_INPUT
:: DEBUG_GAME_INPUT
:: DEBUG_IMGUI_WINDOW
:: DEBUG_CAMERA
SET "debug_configs=/D DEBUG_CAMERA  /D DEBUG_GAME_INPUT"


mkdir Build
pushd Build

:: build tracy 
	:: cl.exe %compiler_flags% /c ..\Source\ThirdParty\tracy\TracyClient.cpp
	:: lib TracyClient.obj

:: build Dear ImGui
	cl.exe %compiler_flags% /c ^
    ..\Source\ThirdParty\imgui\imgui.cpp ^
    ..\Source\ThirdParty\imgui\imgui_draw.cpp ^
    ..\Source\ThirdParty\imgui\imgui_widgets.cpp ^
    ..\Source\ThirdParty\imgui\imgui_tables.cpp ^
    ..\Source\ThirdParty\imgui\imgui_demo.cpp ^
    /I ..\Source\ThirdParty\imgui

	lib imgui.obj imgui_draw.obj imgui_widgets.obj imgui_tables.obj	imgui_demo.obj

:: build {fmt}
::	cl.exe %compiler_flags% /c ..\Source\ThirdParty\fmt\src\format.cc ..\Source\ThirdParty\fmt\src\os.cc /I ..\Source\ThirdParty\fmt\include
::	lib format.obj os.obj	

:: compile and link
	cl.exe %compiler_flags% %debug_configs% ..\Source\main.cpp ^
    ..\Source\Application\*.cpp ^
    ..\Source\Engine\*.cpp ^
    ..\Source\ThirdParty\imgui\imgui_impl_dx11.cpp ^
    ..\Source\ThirdParty\imgui\imgui_impl_win32.cpp ^
    %link_to_libs% ^
    "C:\Program Files (x86)\Microsoft GDK\220602\GRDK\GameKit\Lib\amd64\GameInput.lib"^
    "C:\Program Files (x86)\Microsoft GDK\220602\GRDK\GameKit\Lib\amd64\xgameruntime.lib"^
    /I ..\Source\ThirdParty  ^
    /I ..\Source\ThirdParty\fmt\include  ^
    /I ..\Source\ThirdParty\imgui  ^
    /I ..\Source\ThirdParty\tracy ^
    /I "C:\Program Files (x86)\Microsoft GDK\220602\GRDK\GameKit\Include"^
    /link /incremental /DEBUG:FULL /out:luieditor.exe

popd
