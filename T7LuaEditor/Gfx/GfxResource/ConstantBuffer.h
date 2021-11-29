//
// Created by coxtr on 11/23/2021.
//

#ifndef T7LUAEDITOR_CONSTANTBUFFER_H
#define T7LUAEDITOR_CONSTANTBUFFER_H
#include "Bindable.h"

template<typename CbufType>
class ConstantBuffer : public Bindable {
public:
    ConstantBuffer(Gfx& gfx, const CbufType vtx)
    {
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = sizeof(CbufType);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        HRESULT res = GetDevice(gfx)->CreateBuffer(&bufferDesc, NULL, cbuf.GetAddressOf());

        assert(SUCCEEDED(res));
    }

    void Bind(Gfx& gfx) override
    {
        GetContext(gfx)->VSSetConstantBuffers(0, 1, cbuf.GetAddressOf());
    }

    void Update(Gfx& gfx, const CbufType* newData)
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        CbufType* dataPtr;
        unsigned int bufferNum;
        HRESULT res = GetContext(gfx)->Map(cbuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        assert(SUCCEEDED(res));
        dataPtr = (CbufType*)mappedResource.pData;
        memcpy(dataPtr, newData, sizeof(CbufType));
        GetContext(gfx)->Unmap(cbuf.Get(), 0);
        Bind(gfx); // TODO might be ok to remove
    } // for now, just get the matrices from gfx and upload to GPU
private:
    wrl::ComPtr<ID3D11Buffer> cbuf;
    D3D11_BUFFER_DESC bufferDesc;
};


#endif //T7LUAEDITOR_CONSTANTBUFFER_H
