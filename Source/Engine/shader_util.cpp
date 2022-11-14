//
// Created by coxtr on 11/22/2021.
//
#include "shader_util.h"
#include "logging.h"
#include <cassert>
#include <d3dcompiler.h>
#include <fstream>
#include <iterator>

using namespace Microsoft::WRL;

bool shader_compile_disk(fs::path shader_path, const char *szEntrypoint,
                         const char *szTarget, ID3D10Blob **pBlob) {
  ID3D10Blob *pErrorBlob = nullptr;

  std::ifstream src_file(shader_path);
  std::string shader_code{std::istreambuf_iterator<char>(src_file),
                          std::istreambuf_iterator<char>()};

  HRESULT r =
      D3DCompile(shader_code.data(), shader_code.size(),
                 shader_path.string().c_str(), nullptr, nullptr, szEntrypoint,
                 szTarget, D3DCOMPILE_ENABLE_STRICTNESS, 0, pBlob, &pErrorBlob);

  if (FAILED(r)) {
    if (pErrorBlob) {
      LOG_FATAL("SHADER ERROR: {}", (char *)pErrorBlob->GetBufferPointer());
    }
    return false;
  }
  return true;
}

bool shader_compile(const char *szShader, const char *szEntrypoint,
                    const char *szTarget, ID3D10Blob **pBlob) {
  ID3D10Blob *pErrorBlob = nullptr;

  auto hr =
      D3DCompile(szShader, strlen(szShader), 0, nullptr, nullptr, szEntrypoint,
                 szTarget, D3DCOMPILE_ENABLE_STRICTNESS, 0, pBlob, &pErrorBlob);
  if (FAILED(hr)) {
    if (pErrorBlob) {
      std::string error_str;
      error_str.resize(pErrorBlob->GetBufferSize() + 1);
      memcpy(error_str.data(), pErrorBlob->GetBufferPointer(),
             pErrorBlob->GetBufferSize());
      LOG_FATAL("SHADER ERROR: {}", error_str);
    }
    return false;
  }
  return true;
}

void build_vertex_shader(ID3D11Device *device, const char *path,
                         D3D11_INPUT_ELEMENT_DESC *il, int il_size,
                         ID3D11VertexShader **shaderOut,
                         ID3D11InputLayout **inputLayout) {
  ComPtr<ID3DBlob> bytecode;
  bool compileSuccess =
      shader_compile_disk(path, "vs_main", "vs_5_0", &bytecode);
  assert(compileSuccess == true);
  device->CreateVertexShader(bytecode->GetBufferPointer(),
                             bytecode->GetBufferSize(), nullptr, shaderOut);
  /*
   D3D11_INPUT_ELEMENT_DESC il[] = {
       {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
   D3D11_INPUT_PER_VERTEX_DATA, 0},
       {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
   D3D11_INPUT_PER_VERTEX_DATA, 0},
       {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28,
   D3D11_INPUT_PER_VERTEX_DATA, 0}
   };
   */

  device->CreateInputLayout(il, il_size, bytecode->GetBufferPointer(),
                            bytecode->GetBufferSize(), inputLayout);
}

void build_vertex_shader(ID3D11Device *device, const char *path,
                         ID3D11VertexShader **shaderOut,
                         ID3D11InputLayout **inputLayout) {
  ComPtr<ID3DBlob> bytecode;
  bool compileSuccess =
      shader_compile_disk(path, "vs_main", "vs_5_0", &bytecode);
  assert(compileSuccess == true);
  device->CreateVertexShader(bytecode->GetBufferPointer(),
                             bytecode->GetBufferSize(), nullptr, shaderOut);
  // I only need one IL
  D3D11_INPUT_ELEMENT_DESC il[] = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
                                    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                                   {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
                                    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                                   {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
                                    28, D3D11_INPUT_PER_VERTEX_DATA, 0}};

  device->CreateInputLayout(il, 3, bytecode->GetBufferPointer(),
                            bytecode->GetBufferSize(), inputLayout);
}

void build_pixel_shader(ID3D11Device *device, const char *path,
                        ID3D11PixelShader **shaderOut) {
  HRESULT res;
  ComPtr<ID3DBlob> pixelShaderBuffer;
  // compile the pixel shader
  bool compileSuccess =
      shader_compile_disk(path, "ps_main", "ps_5_0", &pixelShaderBuffer);
  assert(compileSuccess == true);
  res = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                  pixelShaderBuffer->GetBufferSize(), nullptr,
                                  shaderOut);
  assert(SUCCEEDED(res));
}

void build_vertex_shader_and_input_layout(ID3D11Device *device,
                                          const char *path,
                                          D3D11_INPUT_ELEMENT_DESC *il,
                                          UINT numInputElements,
                                          ID3D11VertexShader **shaderOut,
                                          ID3D11InputLayout **inputLayout) {
  ComPtr<ID3DBlob> bytecode;
  bool compileSuccess =
      shader_compile_disk(path, "vs_main", "vs_5_0", &bytecode);
  assert(compileSuccess == true);
  device->CreateVertexShader(bytecode->GetBufferPointer(),
                             bytecode->GetBufferSize(), nullptr, shaderOut);

  device->CreateInputLayout(il, numInputElements, bytecode->GetBufferPointer(),
                            bytecode->GetBufferSize(), inputLayout);
}
