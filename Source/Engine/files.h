#pragma once
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;


namespace Files {

    // gets the root install
    fs::path get_root();

    // install/Resource folder
    fs::path get_resource_root();

    // get the HLSL source root dir
    fs::path get_shader_root(); 
    
}
