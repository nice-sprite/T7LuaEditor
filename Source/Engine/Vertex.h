//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_VERTEX_H
#define T7LUAEDITOR_VERTEX_H
#include <DirectXMath.h>

struct Vertex {
    DirectX::XMFLOAT3 position; // xyz
    DirectX::XMFLOAT4 color;    // rgba off=12
    DirectX::XMFLOAT2 texCoord; // texture UV coords off = 12 + 16 = 28
};

#endif //T7LUAEDITOR_VERTEX_H
