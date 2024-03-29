//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_SHADERUTIL_H
#define T7LUAEDITOR_SHADERUTIL_H

#include "files.h"
#include "logging.h"
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

bool shader_compile_disk(fs::path shader_src, const char *szEntrypoint,
                         const char *szTarget, ID3D10Blob **pBlob);

bool shader_compile(const char *szShader, const char *szEntrypoint,
                    const char *szTarget, ID3D10Blob **pBlob);

void build_vertex_shader(ID3D11Device *device, const char *path,
                         ID3D11VertexShader **shaderOut,
                         ID3D11InputLayout **inputLayout);

void build_vertex_shader(ID3D11Device *device, const char *path,
                         D3D11_INPUT_ELEMENT_DESC *il, int il_size,
                         ID3D11VertexShader **shaderOut,
                         ID3D11InputLayout **inputLayout);

void build_vertex_shader_and_input_layout(ID3D11Device *device,
                                          const char *path,
                                          D3D11_INPUT_ELEMENT_DESC *il,
                                          UINT numInputElements,
                                          ID3D11VertexShader **shaderOut,
                                          ID3D11InputLayout **inputLayout);

void build_pixel_shader(ID3D11Device *device, const char *path,
                        ID3D11PixelShader **shaderOut);

#endif // T7LUAEDITOR_SHADERUTIL_H
