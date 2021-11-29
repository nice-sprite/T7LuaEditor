//
// Created by coxtr on 11/22/2021.
//

#include "VertexShader.h"

VertexShader::VertexShader(Gfx &gfx, const wchar_t *filepath) {


    bool compileSuccess = CompileShader_Disk(filepath, "vs_main", "vs_5_0", bytecodeBlob.GetAddressOf());
    assert(compileSuccess == true);
    GetDevice(gfx)->CreateVertexShader(bytecodeBlob->GetBufferPointer(),
                                       bytecodeBlob->GetBufferSize(),
                                       nullptr,
                                       vertexShader.GetAddressOf());

}

void VertexShader::Bind(Gfx &gfx) {
    GetContext(gfx)->VSSetShader(vertexShader.Get(), nullptr, 0u);
}

ID3DBlob *VertexShader::GetBlob() const noexcept {
    return bytecodeBlob.Get();
}
