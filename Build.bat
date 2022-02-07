@echo off
setlocal EnableDelayedExpansion

:: Using the following external dependencies
::	https://github.com/fmtlib/fmt
::	https://github.com/ocornut/imgui/tree/docking
::	https://github.com/skypjack/entt
::	https://github.com/nlohmann/json

:: Documentation links
::	https://json.nlohmann.me/api/basic_json/
::	https://fmt.dev/latest/api.html
::	https://github.com/skypjack/entt/wiki
::	https://github.com/ocornut/imgui/wiki -- the imgui docs are kind of bad so you might just be better off looking in the actual code or googling your question

:: /EHsc	enables stdcpp exception handling behavior
:: /TP		treat all files as c++ regardless of file extension
:: /Wn		set compiler warning level
:: /GR-		disables run-time type information. We should never be using RTTI 
:: /Zi		Generates complete debugging information.
:: /c		compile without linking ( for building libs )
:: /utf-8	use the UTF8 execution charset 
SET "compiler_flags=/std:c++17 /W2 /GR- /Zi /EHsc /utf-8 /D UNICODE /D _UNICODE /nologo"
SET "link_to_libs=user32.lib ole32.lib kernel32.lib imgui.lib format.lib d3d11.lib dxgi.lib d3dcompiler.lib d2d1.lib dwrite.lib dxguid.lib"

mkdir Build
pushd Build

:: build Dear ImGui
rem	cl %compiler_flags% /c ..\Source\ThirdParty\imgui\imgui.cpp ..\Source\ThirdParty\imgui\imgui_draw.cpp ..\Source\ThirdParty\imgui\imgui_widgets.cpp ..\Source\ThirdParty\imgui\imgui_tables.cpp /I ..\Source\ThirdParty\imgui
rem	lib imgui.obj imgui_draw.obj imgui_widgets.obj imgui_tables.obj	

rem :: build {fmt}
rem	cl %compiler_flags% /c ..\Source\ThirdParty\fmt\src\format.cc ..\Source\ThirdParty\fmt\src\os.cc /I ..\Source\ThirdParty\fmt\include
rem	lib format.obj os.obj	

:: compile and link
	cl %compiler_flags% ..\Source\T7LuaEditor.cpp ..\Source\Application\*.cpp ..\Source\Engine\*.cpp ..\Source\ThirdParty\imgui\imgui_impl_dx11.cpp ..\Source\ThirdParty\imgui\imgui_impl_win32.cpp %link_to_libs% /I ..\Source\ThirdParty /I ..\Source\ThirdParty\entt\single_include /I ..\Source\ThirdParty\fmt\include /I ..\Source\ThirdParty\imgui /link /out:luieditor.exe

popd
