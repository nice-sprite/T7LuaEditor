#include "debug_lines.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcommon.h>

DebugRenderSystem &DebugRenderSystem::instance() {
  static DebugRenderSystem dbs;
  return dbs;
}

void DebugRenderSystem::init(Renderer &renderer) {
  fs::path debug_line_shader_path =
      Files::get_shader_root() / "debug_line.hlsl";
  renderer.create_vertex_buffer(&vertex_buffer,
                                MaxDebugLines * 2 * sizeof(VertexPosColor));
  renderer.create_vertex_shader(debug_line_shader_path,
                                VertexPosColor::layout(),
                                &line_vertex_shader,
                                &vertex_layout);
  renderer.create_pixel_shader(debug_line_shader_path, &line_pixel_shader);
}

void DebugRenderSystem::clear_debug_lines() { n_lines = 0; }

void DebugRenderSystem::debug_line_vec4(XMVECTOR a,
                                        XMVECTOR b,
                                        XMFLOAT4 color) {
  if (n_lines < MaxDebugLines) {
    XMStoreFloat3(&lines[n_lines].begin, a);
    XMStoreFloat3(&lines[n_lines].end, b);
    lines[n_lines].color = color;
    ++n_lines;
  }
}

void DebugRenderSystem::update_line_vec4(u32 i,
                                         XMVECTOR a,
                                         XMVECTOR b,
                                         XMFLOAT4 color) {
  if (i >= 0 && i < MaxDebugLines) {
    XMStoreFloat3(&lines[i].begin, a);
    XMStoreFloat3(&lines[i].end, b);
    lines[i].color = color;
  }
}

void DebugRenderSystem::update_line_float3(u32 i,
                                           XMFLOAT3 a,
                                           XMFLOAT3 b,
                                           XMFLOAT4 color) {
  if (i >= 0 && i < MaxDebugLines) {
    lines[i].begin = a;
    lines[i].end = b;
    lines[i].color = color;
  }
}

void DebugRenderSystem::debug_ray(ray_cast::Ray ray) {
  auto end = XMVectorAdd(ray.origin, XMVectorScale(ray.direction, 100.f));
  debug_line_vec4(ray.origin, end, colors[Red]);
}

void DebugRenderSystem::update(Renderer &renderer) {
  renderer.update_buffer(
      vertex_buffer.Get(),
      [=](D3D11_MAPPED_SUBRESOURCE &msr) {
        VertexPosColor *vert_mem = (VertexPosColor *)msr.pData;
        for (u32 i = 0u; i < n_lines; ++i) {
          vert_mem[i * 2 + 0] = VertexPosColor{lines[i].begin, lines[i].color};
          vert_mem[i * 2 + 1] = VertexPosColor{lines[i].end, lines[i].color};
        }
      });
}

void DebugRenderSystem::draw(Renderer &renderer) {
  renderer.set_topology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  renderer.set_vertex_buffer(vertex_buffer.GetAddressOf(),
                             1,
                             sizeof(VertexPosColor),
                             0);
  renderer.set_vertex_shader(line_vertex_shader.Get());
  renderer.set_pixel_shader(line_pixel_shader.Get());
  renderer.set_input_layout(vertex_layout.Get());
  renderer.draw(2 * n_lines);
}