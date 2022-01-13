# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
add_definitions(-DUNICODE)
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

project ("T7LuaEditor")
 
# Include sub-projects.
add_subdirectory ("T7LuaEditor")


