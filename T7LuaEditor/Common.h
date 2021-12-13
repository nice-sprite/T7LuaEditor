//
// Created by coxtr on 12/12/2021.
//

#ifndef T7LUAEDITOR_COMMON_H
#define T7LUAEDITOR_COMMON_H

#include <wrl/client.h>
#include <d3d11.h>
#include <DirectXMath.h>

#define STRINGIZING(x) #x
#define STR(x) STRINGIZING(x)
#define FILE_LINE __FILE__ ":" STR(__LINE__)
#define DX_ERROR_TITLE "(DX_ERROR): " FILE_LINE
#define DX_ASSERT(expression, message) \
if (!SUCCEEDED(expression)) { \
    MessageBox(NULL, TEXT(DX_ERROR_TITLE), TEXT(message), MB_OK);\
}

namespace wrl = Microsoft::WRL;


#endif //T7LUAEDITOR_COMMON_H
