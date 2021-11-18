# CMakeList.txt : CMake project for T7LuaEditor, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.

add_executable (T7LuaEditor WIN32 "T7LuaEditor.cpp" "T7LuaEditor.h" "Gfx/Gfx.h" "Gfx/Gfx.cpp" "Gfx/WindowUtil.h" "Gfx/WIndowUtil.cpp")
add_definitions(-DUNICODE)
# TODO: Add tests and install targets if needed.

find_package(fmt CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE fmt::fmt)

find_package(simdjson CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE simdjson::simdjson)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(T7LuaEditor PRIVATE imgui::imgui)
