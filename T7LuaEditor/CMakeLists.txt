# CMakeList.txt : CMake project for T7LuaEditor, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (T7LuaEditor WIN32 "T7LuaEditor.cpp" "T7LuaEditor.h" "Engine/Renderer.h" "Engine/Renderer.cpp" "Engine/WindowUtil.h" "Engine/WindowUtil.cpp" "Lua/AnimParser.h" Exceptions/GfxException.cpp Exceptions/GfxException.h Engine/GfxResource/PixelShader.cpp Engine/GfxResource/PixelShader.h Engine/GfxResource/Mesh.cpp Engine/GfxResource/Mesh.h Engine/GfxResource/GraphicsResource.cpp Engine/GfxResource/GraphicsResource.h Engine/GfxResource/Bindable.cpp Engine/GfxResource/Bindable.h Engine/GfxResource/VertexShader.cpp Engine/GfxResource/VertexShader.h Engine/GfxResource/ShaderUtil.h Engine/GfxResource/ShaderUtil.cpp Engine/GfxResource/InputLayout.cpp Engine/GfxResource/InputLayout.h Engine/GfxResource/VertexBuffer.cpp Engine/GfxResource/VertexBuffer.h Engine/Vertex.cpp Engine/Vertex.h Engine/GfxResource/Viewport.cpp Engine/GfxResource/Viewport.h Engine/GfxResource/RenderTarget.cpp Engine/GfxResource/RenderTarget.h Engine/GfxResource/ConstantBuffer.cpp Engine/GfxResource/ConstantBuffer.h Engine/GfxResource/IndexBuffer.cpp Engine/GfxResource/IndexBuffer.h Engine/Camera.cpp Engine/Camera.h Engine/Timer.cpp Engine/Timer.h Engine/SystemInfo.cpp Engine/SystemInfo.h Editor/Widgets/AnimationTimelineSequencer.cpp Editor/Widgets/AnimationTimelineSequencer.h Editor/Widgets/AnimationCurveEdit.cpp Editor/Widgets/AnimationCurveEdit.h Engine/Texture.cpp Engine/Texture.h Common.cpp Common.h Engine/ImGUIManager.cpp Engine/ImGUIManager.h Engine/SceneSystem.cpp Engine/SceneSystem.h 3rdparty/ImGuiFileDialog/ImGuiFileDialog.cpp 3rdparty/ImGuiFileDialog/ImGuiFileDialog.h)
target_compile_features(T7LuaEditor PUBLIC cxx_std_17)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)
# TODO: Add tests and install targets if needed.

find_package(fmt CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE fmt::fmt)

find_package(simdjson CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE simdjson::simdjson)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE imgui::imgui)

find_package(imguizmo CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE imguizmo::imguizmo)