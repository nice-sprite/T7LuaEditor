//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_VERTEXSHADER_H
#define T7LUAEDITOR_VERTEXSHADER_H
#include "../../t7pch.h"
#include "ShaderUtil.h"

void BuildVertexShader(ID3D11Device *device,
                       const wchar_t *path,
                       ID3D11VertexShader **outVS,
                       ID3D11InputLayout **inputLayout);
#endif //T7LUAEDITOR_VERTEXSHADER_H
