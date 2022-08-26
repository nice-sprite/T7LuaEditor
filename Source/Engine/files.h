#pragma once
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;


namespace files {

    // gets the root install
    fs::path get_root();

    // get the HLSL source root dir
    fs::path get_shader_root(); 
    
}
