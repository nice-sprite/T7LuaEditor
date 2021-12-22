//
// Created by coxtr on 11/25/2021.
//

#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(Renderer &gfx, std::vector<unsigned long>& idxs) {
    D3D11_SUBRESOURCE_DATA subresourceData;
    bufDef.Usage = D3D11_USAGE_DEFAULT;
    bufDef.ByteWidth = sizeof(unsigned long) * idxs.size();
    bufDef.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufDef.CPUAccessFlags = 0u;
    bufDef.MiscFlags = 0u;
    bufDef.StructureByteStride = 0u;

    subresourceData.pSysMem = idxs.data();
    subresourceData.SysMemPitch = 0;
    subresourceData.SysMemSlicePitch = 0;
    HRESULT res = GetDevice(gfx)->CreateBuffer(&bufDef, &subresourceData, indexBuffer.GetAddressOf());
    assert(SUCCEEDED(res));
}

void IndexBuffer::Bind(Renderer &gfx) {
    GetContext(gfx)->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}
