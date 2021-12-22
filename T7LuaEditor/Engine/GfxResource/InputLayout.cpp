//
// Created by coxtr on 11/22/2021.
//

#include "InputLayout.h"

InputLayout::InputLayout(Renderer &gfx, const VertexShader &vertexShader) {
    D3D11_INPUT_ELEMENT_DESC layoutDesc[2]{};
    /*
     * TODO: im sure some clever shit here could be done but i wanna keep this bit simple, just match the
    * vertex from vertex.h*/
    layoutDesc[0].SemanticName = "POSITION";
    layoutDesc[0].SemanticIndex = 0;
    layoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    layoutDesc[0].InputSlot = 0;
    layoutDesc[0].AlignedByteOffset = 0;
    layoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layoutDesc[0].InstanceDataStepRate = 0;

    layoutDesc[1].SemanticName = "COLOR";
    layoutDesc[1].SemanticIndex = 0;
    layoutDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    layoutDesc[1].InputSlot = 0;
    layoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    layoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layoutDesc[1].InstanceDataStepRate = 0;

    int numElements = sizeof(layoutDesc) / sizeof(layoutDesc[1]);
    HRESULT res = GetDevice(gfx)->CreateInputLayout(layoutDesc, numElements, vertexShader.GetBufferPtr(),
                                            vertexShader.GetProgramSize(), layout.GetAddressOf());
    assert(SUCCEEDED(res));
}

void InputLayout::Bind(Renderer& gfx)
{
    GetContext(gfx)->IASetInputLayout(layout.Get());
}

