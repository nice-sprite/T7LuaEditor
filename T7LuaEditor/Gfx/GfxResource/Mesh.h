//
// Created by coxtr on 11/21/2021.
//

#ifndef T7LUAEDITOR_MESH_H
#define T7LUAEDITOR_MESH_H

#include "PixelShader.h"
#include "../Vertex.h"
#include <wrl/client.h>

class Renderer;

class Mesh {

public:
    Mesh(Renderer &gfx);

    int UseShader(const char *shaderpath);

private:

    PixelShader shader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    size_t vertexCount, indexCount;
};


#endif //T7LUAEDITOR_MESH_H
