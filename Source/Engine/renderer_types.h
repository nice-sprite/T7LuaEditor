#pragma once
#include "../defines.h"
#include <DirectXMath.h>
#include <array>
#include <d3d11_4.h>

using namespace DirectX;

template <u32 NumElems> struct VertexLayout {
  u32 size = sizeof(D3D11_INPUT_ELEMENT_DESC) * NumElems;
  std::array<D3D11_INPUT_ELEMENT_DESC, NumElems> input_layout;
};

struct VertexPosColorTexcoord {
  XMFLOAT3 pos;
  XMFLOAT4 color;
  XMFLOAT2 texcoord;
  static VertexLayout<3> layout();
};

struct VertexPosColor {
  XMFLOAT3 pos;
  XMFLOAT4 color;
  static VertexLayout<2> layout();
};

struct DebugLine {
  XMFLOAT3 begin;
  XMFLOAT3 end;
  XMFLOAT4 color;
};

__declspec(align(16)) struct PerSceneConsts {
  XMMATRIX modelViewProjection; // 64 bytes
  XMFLOAT4 timeTickDeltaFrame;  // 16 bytes
  XMFLOAT2 viewportSize;        // 8 bytes
  XMFLOAT2 windowSize;          // 8 bytes
};

struct ViewportRegion {
  f32 x, y, w, h;
};
