//
// Created by coxtr on 11/22/2021.
//

#include "VertexShader.h"

VertexShader::VertexShader( Renderer& gfx, const wchar_t* filepath )
{
    bool compileSuccess = Shader_CompileFromDisk( filepath, "vs_main", "vs_5_0", bytecodeBlob.GetAddressOf() );
    assert( compileSuccess == true );
    GetDevice( gfx )->CreateVertexShader( bytecodeBlob->GetBufferPointer(),
        bytecodeBlob->GetBufferSize(),
        nullptr,
        vertexShader.GetAddressOf() );

}

void VertexShader::Bind( Renderer& gfx )
{
    GetContext( gfx )->VSSetShader( vertexShader.Get(), nullptr, 0u );
}

LPVOID VertexShader::GetBufferPtr() const noexcept
{
    return bytecodeBlob->GetBufferPointer();
}

size_t VertexShader::GetProgramSize() const noexcept
{
    return bytecodeBlob->GetBufferSize();
}
