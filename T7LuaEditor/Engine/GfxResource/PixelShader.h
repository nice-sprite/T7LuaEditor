//
// Created by coxtr on 11/21/2021.
//

#ifndef T7LUAEDITOR_PIXELSHADER_H
#define T7LUAEDITOR_PIXELSHADER_H
#include <d3d11.h>
#include "Bindable.h"
#include "ShaderUtil.h"
#include <string>
using DirectX::XMMATRIX;


class PixelShader : public Bindable {
private:
    std::wstring path;
    wrl::ComPtr<ID3D11PixelShader> pixelShader;
public:
    PixelShader(Renderer& gfx, const wchar_t* filepath);
    void Bind(Renderer& gfx) override; // binds the shader to the pipeline
};


#endif //T7LUAEDITOR_PIXELSHADER_H
