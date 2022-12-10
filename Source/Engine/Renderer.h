#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H

#include "../defines.h"
#include "gpu_resources.h"
#include "logging.h"
#include "renderer_types.h"
/*
#include <d2d1_3.h>
#include <d3d11.h>
#include <d3d11_4.h>
#include <dwrite_3.h>
*/
#include "camera.h"
#include "files.h"
#include "shader_util.h"
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;
using namespace DirectX;

class Renderer {

public:
  Renderer();

  ~Renderer();

  Renderer(const Renderer &Gfx) = delete;

  Renderer &operator=(const Renderer &) = delete;

  void init(HWND hwnd, u32 width, u32 height);

  void set_and_clear_backbuffer();

  void present();

  void resize_swapchain_backbuffer(int width, int height, bool minimized);

  void imgui_frame_begin();

  void imgui_frame_end();

  // creates the grid plane
  void create_world_grid();

  void create_world_grid_horizon();

  void create_vertex_buffer(ID3D11Buffer **out_buffer, u32 size_bytes);
  void create_index_buffer(ID3D11Buffer **out_buffer, u32 num_indices);
  void create_pixel_shader(fs::path src_path, ID3D11PixelShader **out_shader);

  template <u32 NumElems>
  void create_input_layout(VertexLayout<NumElems> il,
                           ID3D11InputLayout **out_layout,
                           ID3DBlob *vertex_shader_blob) {
    device->CreateInputLayout(il.input_layout.data(),
                              NumElems,
                              vertex_shader_blob->GetBufferPointer(),
                              vertex_shader_blob->GetBufferSize(),
                              out_layout);
  }

  template <u32 NumElems>
  void create_vertex_shader(fs::path src_path,
                            VertexLayout<NumElems> il,
                            ID3D11VertexShader **out_shader,
                            ID3D11InputLayout **out_layout) {
    bool shader_exists = Files::file_exists(src_path);
    if (shader_exists) {
      LOG_INFO("loading shader {}", src_path.string());
      ComPtr<ID3DBlob> bytecode;
      bool compiled = shader_compile_disk(src_path.string().c_str(),
                                          "vs_main",
                                          "vs_5_0",
                                          &bytecode);
      Q_ASSERTMSG(compiled, "shader failed to compile!");
      if (compiled) {
        HRESULT hresult =
            device->CreateVertexShader(bytecode->GetBufferPointer(),
                                       bytecode->GetBufferSize(),
                                       nullptr,
                                       out_shader);
        LOG_COM(hresult);
        create_input_layout(il, out_layout, bytecode.Get());
      }
    } else {
      LOG_WARNING("shader {} does not exist!", src_path.string());
    }
  }
  void set_vertex_buffer(ID3D11Buffer **buffers,
                         u32 buff_count,
                         u32 stride,
                         u32 offset);

  void set_index_buffer(ID3D11Buffer *buffer);

  void set_pixel_shader(ID3D11PixelShader *shader);

  void set_vertex_shader(ID3D11VertexShader *shader);

  void set_input_layout(ID3D11InputLayout *il);

  void set_topology(D3D11_PRIMITIVE_TOPOLOGY topo);

  template <typename Fn> void update_buffer(ID3D11Buffer *buffer, Fn fn) {
    D3D11_MAPPED_SUBRESOURCE msr{};
    context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    fn(msr);
    context->Unmap(buffer, 0);
  }

  template <typename UpdateFn> void update_shader_constants(UpdateFn fn) {
    fn(scene_consts);
    update_buffer(scene_constant_buffer.Get(),
                  [=](D3D11_MAPPED_SUBRESOURCE &msr) {
                    memcpy(msr.pData, &scene_consts, sizeof(scene_consts));
                  });
  }

  void draw_indexed(u32 num_indices);
  void draw(u32 vert_count);
  /* Selection Rect API
   * allows drawing of 3d selection rectangles to highlight
   * selected quads/items.
   * */
  // TODO change to take Z into account, so rotated elements have perfectly
  // wrapped selection rects
  // void draw_selection_rect();
  // void add_selection_rect(float left, float right, float top, float bottom);
  // void set_selection_rect(int index, float left, float right, float top,
  // float bottom);

  f32 get_aspect_ratio();

private:
  bool init_gfx();

  bool init_imgui();

  void create_backbuffer_view();

  void reset_backbuffer_views();

  void scene_pick(float x, float y);

public:
  float width, height;

  ComPtr<ID3D11Device5> device;
  ComPtr<ID3D11DeviceContext4> context;

  // used for drawing tools and selections
  // consider reserving a spot in an different vbuf?
  //  XMFLOAT4 selection_border_color{0.0, 0.0, 1.0, 1.0};
  //  XMFLOAT4 selection_inner_color{0.48f, 0.75f, 0.95f, 1.f};
  //  float selection_border_thickness = 3.f;
  // static constexpr size_t MaxSelections = 32;
  // static constexpr size_t SelectionsVertexSize =
  // sizeof(VertexPosColorTexcoord) * MaxSelections * 5 * 4; static constexpr
  // size_t SelectionsIndexSize = sizeof(VertexPosColorTexcoord) * MaxSelections
  // * 5 * 6; SelectionArea selections[MaxSelections];
  size_t selection_count = 0;
  ComPtr<ID3D11Buffer> selection_vertex_buffer;
  ComPtr<ID3D11Buffer> selection_index_buffer;

  //Camera camera;
  PerSceneConsts scene_consts;
  ComPtr<ID3D11Buffer> scene_constant_buffer;

  /* debug lines */
  static constexpr size_t MAX_DEBUG_LINES = 1024u;
  // DebugLine debug_lines[MAX_DEBUG_LINES];
  // uint32_t debug_line_count;
  // ComPtr<ID3D11Buffer> debug_line_vbuf;
  // ComPtr<ID3D11InputLayout> debug_line_il;

private:
  ComPtr<IDXGISwapChain4> swapChain;
  // D3D
  D3D11_VIEWPORT viewport{};
  ComPtr<ID3D11BlendState> alphaBlendState;
  ComPtr<ID3D11RasterizerState2> rasterizerState;
  ComPtr<ID3D11RenderTargetView> rtv;
  ComPtr<IDXGIFactory7> dxgiFactory;
  ComPtr<ID3D11SamplerState> gridSS;
  ComPtr<ID3D11Texture2D> depth_stencil_texture;
  ComPtr<ID3D11DepthStencilView> depth_stencil_view;

  DirectX::XMFLOAT4 clear_color;
  HWND hwnd;
};

#endif // !GFX_HELPERS_H
