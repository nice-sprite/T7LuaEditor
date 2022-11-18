#pragma once
#include "ray_cast.h"
#include "renderer.h"
#include "renderer_types.h"
#include <DirectXMath.h>
#include <array>
#include <d3d11.h>
struct Renderer;

constexpr unsigned int MaxDebugLines = 1024;
enum DebugColors { Red, Green, Blue, Pink, Max };

constexpr std::array<XMFLOAT4, DebugColors::Max> colors = {
    XMFLOAT4(1.0, 0, 0, 1.0), XMFLOAT4(0, 1.0, 0, 1.0),
    XMFLOAT4(0, 0, 1.0, 1.0), XMFLOAT4(0.95, 0.003, 1.0, 1.0)};

// allow to draw shapes for debugging help
struct DebugRenderSystem {

  DebugLine lines[MaxDebugLines];
  u32 n_lines = 0;

  void init(Renderer &renderer);

  void debug_line_vec4(XMVECTOR a, XMVECTOR b, XMFLOAT4 color);

  void update_line_vec4(u32 i, XMVECTOR a, XMVECTOR b, XMFLOAT4 color);

  void update_line_float3(u32 i, XMFLOAT3 a, XMFLOAT3 b, XMFLOAT4 color);

  void debug_ray(ray_cast::Ray ray);

  void clear_debug_lines();

  // update GPU buffers
  void update(Renderer &renderer);
  void draw(Renderer &renderer);

  static DebugRenderSystem &instance();

  ComPtr<ID3D11Buffer> vertex_buffer;
  ComPtr<ID3D11Buffer> index_buffer; // maybe unused
  ComPtr<ID3D11InputLayout> vertex_layout;
  ComPtr<ID3D11VertexShader> line_vertex_shader;
  ComPtr<ID3D11PixelShader> line_pixel_shader;
};
