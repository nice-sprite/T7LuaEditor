//
// Created by coxtr on 11/21/2021.
//

#include "Mesh.h"

Mesh::Mesh(Gfx &gfx)
        : shader(gfx, L"C:\\Users\\coxtr\\source\\repos\\T7LuaEditor\\T7LuaEditor\\ShaderSrc\\test.hlsl") {
    HRESULT result;
    Vertex* verts;
    using ul = unsigned long;
    ul* idx;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;

    vertexCount = 3;
    indexCount = 3;
    verts = new Vertex[vertexCount];
    idx = new ul[indexCount];

    verts[0].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f); // bottom left
    verts[0].color = DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f);

    verts[1].position = DirectX::XMFLOAT3(0.0, 1.0f, 0.0f); // top middle
    verts[1].color = DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f);

    verts[2].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f); // bottom right
    verts[2].color = DirectX::XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f);
    idx[0] = 0;
    idx[1] = 1;
    idx[2] = 2;

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = verts;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

}

int Mesh::UseShader(const char *shaderpath) {
    return 0;
}
