cmake_minimum_required(VERSION 3.22)

project(Priscilla)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)


if(MSVC)
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS")
    add_compile_options(/W2 /MP /GR- /Zi /EHsc /utf-8)
else()
endif()

add_compile_definitions(NOMINMAX TRACY_ENABLE _CRT_SECURE_NO_WARNINGS)
include_directories(
  "Source/ThirdParty/"
  "Source/ThirdParty/fmt/include"
  "Source/ThirdParty/imgui"
  "Source/ThirdParty/tracy"
  "Source/ThirdParty/freetype/include"
)



add_library(tracy STATIC "Source/ThirdParty/tracy/TracyClient.cpp")

add_library(fmt STATIC "Source/ThirdParty/fmt/src/format.cc" "Source/ThirdParty/fmt/src/os.cc" "Source/ThirdParty/fmt/src/os.cc")

add_library(imgui STATIC
  "Source/ThirdParty/imgui/imgui.cpp"
  "Source/ThirdParty/imgui/imgui_draw.cpp"
  "Source/ThirdParty/imgui/imgui_widgets.cpp"
  "Source/ThirdParty/imgui/imgui_tables.cpp"
  "Source/ThirdParty/imgui/imgui_demo.cpp"
)

file(GLOB ENGINE_SRC "Source/Engine/*.cpp")
file(GLOB APP_SRC "Source/Application/*.cpp")
set(IMGUI_SRC "Source/ThirdParty/imgui/imgui_impl_dx11.cpp" "Source/ThirdParty/imgui/imgui_impl_win32.cpp")
add_executable(Priscilla WIN32 "Source/main.cpp" ${ENGINE_SRC} ${APP_SRC} ${IMGUI_SRC})

set(FREETYPE_DEBUG_STATIC ../bin/freetype/objs/x64/DebugStatic/freetype)
set(FREETYPE_RELEASE_STATIC ../bin/freetype/objs/x64/ReleaseStatic/freetype)


target_link_libraries(Priscilla tracy fmt imgui d3d11.lib dxgi.lib d3dcompiler.lib dxguid.lib dwmapi.lib ole32.lib kernel32.lib xinput.lib ${FREETYPE_DEBUG_STATIC} harfbuzz)
