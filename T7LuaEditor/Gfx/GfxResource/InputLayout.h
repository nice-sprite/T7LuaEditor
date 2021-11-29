//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_INPUTLAYOUT_H
#define T7LUAEDITOR_INPUTLAYOUT_H

#include "Bindable.h"
#include <d3d11.h>
#include "VertexShader.h"

class InputLayout : public Bindable {
    wrl::ComPtr<ID3D11InputLayout> layout;

public:

    InputLayout(Gfx &gfx, const VertexShader &vertexShader);

    void Bind(Gfx &gfx) override;
};


#endif //T7LUAEDITOR_INPUTLAYOUT_H
