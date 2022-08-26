#include "files.h"


// gets the root install 
fs::path files::get_root() {
    TCHAR buf[MAX_PATH]{};
    GetModuleFileName(nullptr, buf, MAX_PATH);
    fs::path root_path = buf;

    return root_path.parent_path().parent_path();
}

// get the HLSL source root dir
fs::path files::get_shader_root() {

    return files::get_root() / "Source/HLSL";
}
    