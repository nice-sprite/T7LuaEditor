//
// Created by coxtr on 11/22/2021.
//

#include "VertexBuffer.h"

// we only use one vertex layout so...
VertexBuffer::VertexBuffer(Gfx &gfx, std::vector<Vertex>& vertexList)
{
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0u;
    bufferDesc.MiscFlags = 0u;
    bufferDesc.ByteWidth = sizeof(Vertex)* vertexList.size();
    bufferDesc.StructureByteStride = 0u;

    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertexList.data();
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    HRESULT res = GetDevice(gfx)->CreateBuffer(&bufferDesc, &vertexData, vertexBuffer.GetAddressOf());
    assert(SUCCEEDED(res));
}

void VertexBuffer::Bind(Gfx &gfx) {
    unsigned int stride = sizeof(Vertex);
    unsigned int offset = 0;
    GetContext(gfx)->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
}
