# CMakeList.txt : CMake project for T7LuaEditor, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resource.rc" )
# Add source to this project's executable.
add_executable(T7LuaEditor
        WIN32
        "Application/Application.h"
        "Application/Application.cpp"
        "Engine/Input.h"
        "Engine/Input.cpp"
        "Engine/Renderer.cpp"
        "Engine/Renderer.h"
        "Engine/WindowUtil.cpp"
        "Engine/WindowUtil.h"
        "Lua/AnimParser.h"
        "T7LuaEditor.cpp"
        "T7LuaEditor.h"
        3rdparty/ImGuiFileDialog/ImGuiFileDialog.cpp
        3rdparty/ImGuiFileDialog/ImGuiFileDialog.h
        Common.cpp
        Common.h
        Editor/EditorLayer.cpp
        Editor/EditorLayer.h
        Editor/Widgets/AssetPicker/AssetView.cpp
        Editor/Widgets/AssetPicker/AssetView.h
        Editor/Widgets/KeyframeEditor/AnimationCurveEdit.cpp
        Editor/Widgets/KeyframeEditor/AnimationCurveEdit.h
        Editor/Widgets/KeyframeEditor/AnimationTimelineSequencer.cpp
        Editor/Widgets/KeyframeEditor/AnimationTimelineSequencer.h
        Engine/Camera.cpp
        Engine/Camera.h
        Engine/GfxResource/ConstantBuffer.cpp
        Engine/GfxResource/ConstantBuffer.h
        Engine/GfxResource/IndexBuffer.cpp
        Engine/GfxResource/IndexBuffer.h
        Engine/GfxResource/PixelShader.cpp
        Engine/GfxResource/PixelShader.h
        Engine/GfxResource/RenderTarget.cpp
        Engine/GfxResource/RenderTarget.h
        Engine/GfxResource/ShaderUtil.cpp
        Engine/GfxResource/ShaderUtil.h
        Engine/GfxResource/VertexBuffer.cpp
        Engine/GfxResource/VertexBuffer.h
        Engine/GfxResource/VertexShader.cpp
        Engine/GfxResource/VertexShader.h
        Engine/GfxResource/Viewport.cpp
        Engine/GfxResource/Viewport.h
        Engine/ImGUIManager.cpp
        Engine/ImGUIManager.h
        Engine/Scene.cpp
        Engine/Scene.h
        Engine/SystemInfo.cpp
        Engine/SystemInfo.h
        Engine/Texture.cpp
        Engine/Texture.h
        Engine/TextureManager.cpp
        Engine/TextureManager.h
        Engine/Timer.cpp
        Engine/Timer.h
        Engine/Vertex.cpp
        Engine/Vertex.h
        Exceptions/GfxException.cpp
        Exceptions/GfxException.h
        ${APP_ICON_RESOURCE_WINDOWS}
        )
target_precompile_headers(T7LuaEditor PUBLIC t7pch.h)
target_compile_features(T7LuaEditor PUBLIC cxx_std_17)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif (MSVC)
# TODO: Add tests and install targets if needed.

find_package(fmt CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE fmt::fmt)

find_package(simdjson CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE simdjson::simdjson)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE imgui::imgui)

find_package(imguizmo CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE imguizmo::imguizmo)