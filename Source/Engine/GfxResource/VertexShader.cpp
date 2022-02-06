//
// Created by coxtr on 11/22/2021.
//

#include "VertexShader.h"
#include <wrl/client.h>
#include <cassert>
using namespace Microsoft::WRL;
void build_vertex_shader(ID3D11Device *device,
                       const wchar_t *path,
                       ID3D11VertexShader **outVS,
                       ID3D11InputLayout **inputLayout)
{
    ComPtr<ID3DBlob> bytecode;
    bool compileSuccess = Shader_CompileFromDisk(path, "vs_main", "vs_5_0", &bytecode);
    assert(compileSuccess == true);
    device->CreateVertexShader(bytecode->GetBufferPointer(),
                               bytecode->GetBufferSize(),
                               nullptr,
                               outVS);
    // I only need one IL
    D3D11_INPUT_ELEMENT_DESC il[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}};

    device->CreateInputLayout(il, 3,
                              bytecode->GetBufferPointer(), bytecode->GetBufferSize(), inputLayout);
}
