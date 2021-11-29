# CMakeList.txt : CMake project for T7LuaEditor, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (T7LuaEditor WIN32 "T7LuaEditor.cpp" "T7LuaEditor.h" "Gfx/Gfx.h" "Gfx/Gfx.cpp" "Gfx/WindowUtil.h" "Gfx/WindowUtil.cpp" "Lua/AnimParser.h" Exceptions/GfxException.cpp Exceptions/GfxException.h Gfx/GfxResource/PixelShader.cpp Gfx/GfxResource/PixelShader.h Gfx/GfxResource/Mesh.cpp Gfx/GfxResource/Mesh.h Gfx/GfxResource/GraphicsResource.cpp Gfx/GfxResource/GraphicsResource.h Gfx/GfxResource/Bindable.cpp Gfx/GfxResource/Bindable.h Gfx/GfxResource/VertexShader.cpp Gfx/GfxResource/VertexShader.h Gfx/GfxResource/ShaderUtil.h Gfx/GfxResource/ShaderUtil.cpp Gfx/GfxResource/InputLayout.cpp Gfx/GfxResource/InputLayout.h Gfx/GfxResource/VertexBuffer.cpp Gfx/GfxResource/VertexBuffer.h Gfx/Vertex.cpp Gfx/Vertex.h Gfx/GfxResource/Viewport.cpp Gfx/GfxResource/Viewport.h Gfx/GfxResource/RenderTarget.cpp Gfx/GfxResource/RenderTarget.h Gfx/GfxResource/ConstantBuffer.cpp Gfx/GfxResource/ConstantBuffer.h Gfx/DrawMe.cpp Gfx/DrawMe.h Gfx/GfxResource/IndexBuffer.cpp Gfx/GfxResource/IndexBuffer.h Gfx/Camera.cpp Gfx/Camera.h Gfx/Timer.cpp Gfx/Timer.h Gfx/SystemDeviceInfoManager.cpp Gfx/SystemDeviceInfoManager.h)
target_compile_features(T7LuaEditor PUBLIC cxx_std_17)
# TODO: Add tests and install targets if needed.

find_package(fmt CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE fmt::fmt)

find_package(simdjson CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE simdjson::simdjson)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE imgui::imgui)
