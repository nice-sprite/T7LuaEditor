//
// Created by coxtr on 11/21/2021.
//

#include "PixelShader.h"

PixelShader::PixelShader(Renderer &gfx, const wchar_t *filepath) : path{filepath} {
    HRESULT res;
    wrl::ComPtr<ID3DBlob> pixelShaderBuffer;

    // compile the pixel shader
    bool compileSuccess = Shader_CompileFromDisk(filepath, "ps_main", "ps_5_0", &pixelShaderBuffer);
    assert(compileSuccess == true);

    res = GetDevice(gfx)->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(),
                                            nullptr, pixelShader.GetAddressOf());
    assert(SUCCEEDED(res));
}


void PixelShader::Bind(Renderer &gfx) {
    GetContext(gfx)->PSSetShader(pixelShader.Get(), nullptr, 0);
}
