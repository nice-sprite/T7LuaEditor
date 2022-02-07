#ifndef GPU_RESOURCES_H
#define GPU_RESOURCES_H
#include <d3d11_4.h>

constexpr auto CbufSlot_PerSceneConst = 0u;
constexpr auto  CbufSlot_Transforms = 1u;


template <typename TBuffer>
HRESULT update_constant_buffer(ID3D11DeviceContext *context, int bufferSlot, TBuffer *newData, ID3D11Buffer* buffer)
{
    D3D11_MAPPED_SUBRESOURCE mr;
    TBuffer *dataPtr;
    HRESULT res = context->Map(buffer, bufferSlot, D3D11_MAP_WRITE_DISCARD, 0, &mr);
        assert(SUCCEEDED(res));
        dataPtr = (TBuffer*)mr.pData;
        memcpy(dataPtr, newData, sizeof(TBuffer));
    context->Unmap(buffer, bufferSlot);
    return res;
}

void bind_constant_buffer(ID3D11DeviceContext *context, int bufferSlot, ID3D11Buffer *buffer);

template<typename TBuffer>
HRESULT create_constant_buffer(ID3D11Device* device, TBuffer* data, ID3D11Buffer** ppBuffer)
{
    static_assert(alignof(TBuffer) == 16, "constant buffers must be 16 byte aligned");
    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(TBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    D3D11_MAPPED_SUBRESOURCE rm;
    rm.pData = (void*)data;
    rm.DepthPitch = 0;
    rm.RowPitch = 0;
    HRESULT res = device->CreateBuffer(&bd, (const D3D11_SUBRESOURCE_DATA*)&rm, ppBuffer);
    return res;
}

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

HRESULT create_dynamic_index_buffer(ID3D11Device *device, ID3D11Buffer **ppBuffer, int numIndices);
HRESULT create_dynamic_index_buffer(ID3D11Device *device, ID3D11Buffer **ppBuffer, int *initialData, int numIndices);
void bind_dynamic_index_buffer(ID3D11DeviceContext *context, ID3D11Buffer *buf);
void update_dynamic_index_buffer(ID3D11DeviceContext *context, ID3D11Buffer *buf, int *indexData, size_t numIndices);
void append_dynamic_index_buffer(ID3D11DeviceContext *context, ID3D11Buffer *buf, int *indexData, size_t numIndices, size_t offset);
#endif