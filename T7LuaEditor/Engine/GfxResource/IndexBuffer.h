//
// Created by coxtr on 11/25/2021.
//

#ifndef T7LUAEDITOR_INDEXBUFFER_H
#define T7LUAEDITOR_INDEXBUFFER_H
#include "Bindable.h"
#include <vector>
class IndexBuffer : public Bindable {

public:
    IndexBuffer(Renderer& gfx, std::vector<unsigned long>& idxs);
    void Bind(Renderer& gfx) override;

private:
    wrl::ComPtr<ID3D11Buffer> indexBuffer;
    D3D11_BUFFER_DESC bufDef;
};


#endif //T7LUAEDITOR_INDEXBUFFER_H
