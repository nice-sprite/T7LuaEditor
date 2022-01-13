//
// Created by coxtr on 11/23/2021.
//

#ifndef T7LUAEDITOR_CONSTANTBUFFER_H
#define T7LUAEDITOR_CONSTANTBUFFER_H

constexpr auto CbufSlot_PerSceneConst = 0u;
constexpr auto  CbufSlot_Transforms = 1u;


template <typename TBuffer>
HRESULT UpdateConstantBuffer(ID3D11DeviceContext *context, int bufferSlot, TBuffer *newData, ID3D11Buffer* buffer)
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

void BindConstantBuffer(ID3D11DeviceContext *context, int bufferSlot, ID3D11Buffer *buffer);

template<typename TBuffer>
HRESULT CreateConstantBuffer(ID3D11Device* device, TBuffer* data, ID3D11Buffer** ppBuffer)
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

#endif //T7LUAEDITOR_CONSTANTBUFFER_H
