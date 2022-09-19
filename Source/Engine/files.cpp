#include "files.h"


// gets the root install 
fs::path Files::get_root() {
    TCHAR buf[MAX_PATH]{};
    GetModuleFileName(nullptr, buf, MAX_PATH);
    fs::path root_path = buf;
    return root_path.parent_path().parent_path();
}

// get the HLSL source root dir
fs::path Files::get_shader_root() {
    return Files::get_root() / "Source/HLSL";
}
    
// install/Resource folder
fs::path Files::get_resource_root() {
    return Files::get_root() / "Resource";
}
