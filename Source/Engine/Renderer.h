#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H
#include "gpu_resources.h"
#include "render_graph.h"
/*
#include <d2d1_3.h>
#include <d3d11_4.h>
#include <dwrite_3.h>
*/
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include "Camera.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct VertexPosColorTexcoord
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT2 texcoord;
};

struct DebugLine {
    VertexPosColorTexcoord begin, end;
};

__declspec(align(16))
struct PerSceneConsts
{
    XMMATRIX modelViewProjection; // 64 bytes
    XMFLOAT4 timeTickDeltaFrame; // 16 bytes
    XMFLOAT2 viewportSize; // 8 bytes
    XMFLOAT2 windowSize; // 8 bytes
};


class Renderer
{

public:
    explicit Renderer(HWND hwnd, float _width, float _height);
    ~Renderer();
    Renderer(const Renderer &Gfx) = delete;
    Renderer &operator=(const Renderer &) = delete;
    
    void set_and_clear_backbuffer();
    void present();
    void resize_swapchain_backbuffer(int width, int height, bool minimized);
    void imgui_frame_begin();
    void imgui_frame_end();

    void add_debug_line(XMFLOAT3 begin, XMFLOAT3 end, XMFLOAT4 color);
    void add_debug_line_from_vector(XMVECTOR begin, XMVECTOR end, XMFLOAT4 color);
    void clear_debug_lines();
    void draw_debug_lines();
    void set_debug_line(unsigned int i, XMFLOAT3 begin, XMFLOAT3 end, XMFLOAT4 color);
    void set_debug_line_from_vector(unsigned int i, XMVECTOR begin, XMVECTOR end, XMFLOAT4 color);
private:
    bool initialize_d3d();
    bool initialize_imgui();
    void create_backbuffer_view();
    void reset_backbuffer_views();

    void scene_pick(float x, float y);

public: 
    static constexpr auto DEFAULT_SHADER = L"C:\\Users\\nice_sprite\\source\\repos\\Priscilla\\Source\\HLSL\\TexturedQuad.hlsl";
    float width, height;
    RenderGraph renderGraph;

    ComPtr<ID3D11Device5> device;
    ComPtr<ID3D11DeviceContext4> context;


/*this will probably move but for now keep it here for simplicity*/
    ComPtr<ID3D11Buffer> scene_vertex_buffer;
    ComPtr<ID3D11Buffer> scene_index_buffer;
    ComPtr<ID3D11InputLayout> vtx_pos_color_tex_il;
    ComPtr<ID3D11VertexShader> scene_vertex_shader;
    ComPtr<ID3D11PixelShader> scene_pixel_shader;

    Camera camera;
    PerSceneConsts       scene_consts;
    ComPtr<ID3D11Buffer> scene_constant_buffer;

    /* debug lines */
    static constexpr size_t MAX_DEBUG_LINES = 1024u;
    DebugLine debug_lines[MAX_DEBUG_LINES];
    uint32_t debug_line_count;
    ComPtr<ID3D11Buffer> debug_line_vbuf;
    ComPtr<ID3D11InputLayout> debug_line_il;


private:

    ComPtr<IDXGISwapChain4> swapChain;
    // D3D
    D3D11_VIEWPORT viewport{};
    ComPtr<ID3D11BlendState> alphaBlendState;
    ComPtr<ID3D11RasterizerState2> rasterizerState;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<IDXGIFactory7> dxgiFactory;
    ComPtr<ID3D11SamplerState> gridSS;

    /**/

    DirectX::XMFLOAT4 clearColor;
    HWND hwnd;
};

#endif // !GFX_HELPERS_H
