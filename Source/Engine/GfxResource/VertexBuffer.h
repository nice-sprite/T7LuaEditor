//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_VERTEXBUFFER_H
#define T7LUAEDITOR_VERTEXBUFFER_H
#include "../Vertex.h"
#include <d3d11_4.h>

// reserveSize is the number of VertexType to reserve space for
template<typename VertexType>
HRESULT create_dynamic_vertex_buffer(ID3D11Device* device, ID3D11Buffer** ppBuffer, int reserveSize) 
{
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = sizeof(VertexType) * reserveSize;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    auto hres = device->CreateBuffer(&bd, nullptr, ppBuffer);
    return hres;
}

template<typename VertexType>
HRESULT create_dynamic_vertex_buffer(ID3D11Device* device, ID3D11Buffer** ppBuffer, VertexType* initialData, int reserveSize) 
{
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = sizeof(VertexType) * reserveSize;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = initialData;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    auto hres = device->CreateBuffer(&bd, &data, ppBuffer);
    return hres;
}

template<typename VertexType>
void bind_dynamic_vertex_buffers(ID3D11DeviceContext* context, ID3D11Buffer** buf)
{
    UINT stride = sizeof(VertexType);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, buf, &stride, &offset);
}

// updates the entire vertex buffer, discards previous contents
template<typename VertexType> 
void update_dynamic_vertex_buffer(ID3D11DeviceContext* context, ID3D11Buffer* buf, VertexType* vertexData, size_t vertCount)
{
    D3D11_MAPPED_SUBRESOURCE msr{};
    context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

    memcpy(msr.pData, vertexData, vertCount * sizeof(VertexType));

    context->Unmap(buf, 0);
}

template<typename VertexType>
void append_dynamic_vertex_buffer(ID3D11DeviceContext* context, ID3D11Buffer* buf, VertexType* vertexData, size_t vertCount, size_t offset)
{

    D3D11_MAPPED_SUBRESOURCE msr{};
    context->Map(buf, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &msr); // we are telling the device driver we won't overwrite data, only add

    VertexType* gpuVerts = (VertexType*)msr.pData;

    for (int i = 0; i < vertCount; ++i)
    {
        gpuVerts[i + offset] = vertexData[i];
    }
    context->Unmap(buf, 0);

}

#endif //T7LUAEDITOR_VERTEXBUFFER_H
