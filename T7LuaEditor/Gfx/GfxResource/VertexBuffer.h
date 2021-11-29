//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_VERTEXBUFFER_H
#define T7LUAEDITOR_VERTEXBUFFER_H
#include "Bindable.h"
#include "../Vertex.h"
#include <DirectXMath.h>
#include <vector>
class VertexBuffer : public Bindable {

public:

    VertexBuffer(Gfx& gfx, std::vector<Vertex>& vertexList);
    void Bind(Gfx& gfx) override;

private:
    wrl::ComPtr<ID3D11Buffer> vertexBuffer;
    D3D11_BUFFER_DESC bufferDesc;
};


#endif //T7LUAEDITOR_VERTEXBUFFER_H
