//
// Created by coxtr on 11/21/2021.
//

#ifndef T7LUAEDITOR_PIXELSHADER_H
#define T7LUAEDITOR_PIXELSHADER_H
#include "../../t7pch.h"
#include "ShaderUtil.h"

void BuildPixelShader(ID3D11Device *device, const wchar_t *filepath, ID3D11PixelShader **pixelShader);

#endif //T7LUAEDITOR_PIXELSHADER_H
