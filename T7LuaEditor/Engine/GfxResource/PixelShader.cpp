//
// Created by coxtr on 11/21/2021.
//

#include "PixelShader.h"

void BuildPixelShader(ID3D11Device* device, const wchar_t* filepath, ID3D11PixelShader** pixelShader)
{
    HRESULT res;
    ComPtr<ID3DBlob> pixelShaderBuffer;

    // compile the pixel shader
    bool compileSuccess = Shader_CompileFromDisk(filepath, "ps_main", "ps_5_0", &pixelShaderBuffer);
    assert(compileSuccess == true);

    res = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(),
                                            nullptr, pixelShader);
    assert(SUCCEEDED(res));
}
