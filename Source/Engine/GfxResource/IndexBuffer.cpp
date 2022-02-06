//
// Created by coxtr on 11/25/2021.
//

#include "IndexBuffer.h"


HRESULT create_dynamic_index_buffer(ID3D11Device* device, ID3D11Buffer** ppBuffer, int numIndices) 
{
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = numIndices;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    auto hres = device->CreateBuffer(&bd, nullptr, ppBuffer);
    return hres;
}

HRESULT create_dynamic_index_buffer(ID3D11Device* device, ID3D11Buffer** ppBuffer, int* initialData, int numIndices) 
{
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = sizeof(int) * numIndices;  
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
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


void bind_dynamic_index_buffer(ID3D11DeviceContext* context, ID3D11Buffer* buf)
{
    context->IASetIndexBuffer(buf, DXGI_FORMAT_R32_UINT, 0);
}

// updates the entire vertex buffer, discards previous contents
void bind_dynamic_index_buffer(ID3D11DeviceContext* context, ID3D11Buffer* buf, int* indexData, size_t numIndices)
{
    D3D11_MAPPED_SUBRESOURCE msr{};
    context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

    memcpy(msr.pData, indexData, numIndices * sizeof(int));

    context->Unmap(buf, 0);
}

void append_dynamic_index_buffer(ID3D11DeviceContext* context, ID3D11Buffer* buf, int* indexData, size_t numIndices, size_t offset)
{

    D3D11_MAPPED_SUBRESOURCE msr{};
    context->Map(buf, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &msr); // we are telling the device driver we won't overwrite data, only add

    int* gpuIndices = (int*)msr.pData;

    for (int i = 0; i < numIndices; ++i)
    {
        gpuIndices[i + offset] = indexData[i];
    }
    context->Unmap(buf, 0);
}


void update_dynamic_index_buffer(ID3D11DeviceContext *context, ID3D11Buffer *buf, int *indexData, size_t numIndices)
{

    D3D11_MAPPED_SUBRESOURCE msr{};
    context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); // we are telling the device driver we won't overwrite data, only add

    int* gpuIndices = (int*)msr.pData;

    for (int i = 0; i < numIndices; ++i)
    {
        gpuIndices[i] = indexData[i];
    }
    context->Unmap(buf, 0);

}



