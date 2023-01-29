#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H

#include "../defines.h"
#include "files.h"
#include "gpu_resources.h"
#include "logging.h"
#include "renderer_types.h"
#include "shader_util.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include "math.h"

using namespace Microsoft::WRL;
using namespace DirectX;

template<typename T>
void safe_release(T** com_object) {
  if(*com_object != nullptr) {
    (*com_object)->Release();
    *com_object = nullptr;
  }
}
  
struct RenderToTexture {
  ComPtr<ID3D11Texture2D> render_target; // TODO replace with Texture2D
  ComPtr<ID3D11RenderTargetView> render_target_view;
  ComPtr<ID3D11ShaderResourceView> srv;
};

class Renderer {

public:

  Renderer(HWND hwnd, i32 width, i32 height);

  ~Renderer();

  Renderer(const Renderer &Gfx) = delete;

  Renderer &operator=(const Renderer &) = delete;

  /* Backbuffer/Swapchain management */
  void backbuffer_clear(Float4 clear_color);
  void backbuffer_bind();
  void backbuffer_resize(int width, int height, bool minimized);
  void backbuffer_view_create();
  void backbuffer_view_reset();
  f32 backbuffer_aspect_ratio();

  void present();


  /* Shader management */
  void pixel_shader_create(fs::path src_path, ID3D11PixelShader **out_shader);
  void pixel_shader_bind(ID3D11PixelShader *shader);

  template <u32 NumElems>
  void input_layout_create(VertexLayout<NumElems> il,
                           ID3D11InputLayout **out_layout,
                           ID3DBlob *vertex_shader_blob) {
    device->CreateInputLayout(il.input_layout.data(),
                              NumElems,
                              vertex_shader_blob->GetBufferPointer(),
                              vertex_shader_blob->GetBufferSize(),
                              out_layout);
  }

  void input_layout_bind(ID3D11InputLayout *il);

  template <u32 NumElems>
  void vertex_shader_create(fs::path src_path,
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
        input_layout_create(il, out_layout, bytecode.Get());
      }
    } else {
      LOG_WARNING("shader {} does not exist!", src_path.string());
    }
  }

  void vertex_shader_bind(ID3D11VertexShader *shader);

  /* Buffer management */
  void vertex_buffer_create(ID3D11Buffer **out_buffer, u32 size_bytes);
  void vertex_buffer_bind(ID3D11Buffer **buffers, u32 buff_count, u32 stride, u32 offset);

  void index_buffer_create(ID3D11Buffer **out_buffer, u32 num_indices);
  void index_buffer_bind(ID3D11Buffer *buffer);

  template <typename Fn> void update_buffer(ID3D11Buffer *buffer, Fn fn) {
    D3D11_MAPPED_SUBRESOURCE msr{};
    context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    fn(msr);
    context->Unmap(buffer, 0);
  }


  void set_topology(D3D11_PRIMITIVE_TOPOLOGY topo);

  /* Texture management */
  
  /* creates a texture
   * returns true if success, false otherwise
   */
  bool texture_create(TextureParams const &params, Texture2D &out_texture);
  void texture_update_subregion(Texture2D& texture, u32 subresource, D3D11_BOX* region, void* src_data, u32 src_pitch, u32 src_depth_pitch = 0);
  void texture_bind(Texture2D * texture);

  // use this to update the *whole* Texture2D,
  // the original contents will be deleted
  template <typename UpdateFn>
  void texture_update(Texture2D &texture, UpdateFn fn) {
    D3D11_MAPPED_SUBRESOURCE msr{};
    HRESULT map_result = context->Map(texture.texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    LOG_COM(map_result);

    if(!SUCCEEDED(map_result)) return;

    fn((u8 *)msr.pData, msr.RowPitch);
    context->Unmap(texture.texture.Get(), 0);
  }

  /* Render Texture management */
  void render_texture_bind();
  void render_texture_clear(Float4 clear_color);
  void render_texture_create();
  void render_texture_resize(f32 w, f32 h);

  /* implement a stack for push/popping render textures(?) */
  void render_texture_push(RenderToTexture* rtv);
  void render_texture_pop();

  /* submit draw calls */
  void draw_indexed(u32 num_indices);
  void draw(u32 vert_count);

private:

  /* Creates dx11 device, swapchain, and objects for rendering to `hwnd`
   * swapchain dimensions will be `width` x `height`
   * */
  bool d3d11_init(HWND hwnd, i32 width, i32 height);

public:

  f32 width, height;

  ComPtr<ID3D11Device5> device;
  ComPtr<ID3D11DeviceContext4> context;

  PerSceneConsts scene_consts;
  ComPtr<ID3D11Buffer> scene_constant_buffer;

  void draw_fullscreen_quad();

  void set_viewport(ViewportRegion viewport);


private:
  /* TODO GfxState */
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

  ComPtr<ID3D11PixelShader> ps_fullscreen_quad;
  ComPtr<ID3D11VertexShader> vs_fullscreen_quad;
  ComPtr<ID3D11InputLayout> fullscreen_quad_il;


  DirectX::XMFLOAT4 clear_color;
  HWND hwnd;
};

#endif // !GFX_HELPERS_H
