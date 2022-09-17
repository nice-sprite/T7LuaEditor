#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H
#include "gpu_resources.h"
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

// a rectangle describing the bounds of a selection and 
struct SelectionArea {
    float left, right, top, bottom;
};

void query_selection_area(SelectionArea selection);

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
    void set_debug_line_color(unsigned int i, XMFLOAT4 color);
    
    // draws the grid plane
    void create_world_grid();
    void create_world_grid_horizon();

    /* Selection Rect API
     * allows drawing of 3d selection rectangles to highlight 
     * selected quads/items.
     * */
    // TODO change to take Z into account, so rotated elements have perfectly wrapped selection rects
    void draw_selection_rect();
    void add_selection_rect(float left, float right, float top, float bottom);
    void set_selection_rect(int index, float left, float right, float top, float bottom);

    // takes the 2d screen coordinates of where the box should be 
    void imgui_draw_screen_rect(float left, float right, float top, float bottom);
private:
    bool initialize_d3d();
    bool initialize_imgui();
    void create_backbuffer_view();
    void reset_backbuffer_views();

    void scene_pick(float x, float y);

public: 
    float width, height;

    ComPtr<ID3D11Device5> device;
    ComPtr<ID3D11DeviceContext4> context;


/*this will probably move but for now keep it here for simplicity*/
    ComPtr<ID3D11Buffer> scene_vertex_buffer;
    ComPtr<ID3D11Buffer> scene_index_buffer;
    ComPtr<ID3D11InputLayout> vtx_pos_color_tex_il;
    ComPtr<ID3D11VertexShader> scene_vertex_shader;
    ComPtr<ID3D11PixelShader> scene_pixel_shader;

    // used for drawing tools and selections
    // consider reserving a spot in an different vbuf?
    XMFLOAT4 selection_border_color{0.0, 0.0, 1.0, 1.0};
    XMFLOAT4 selection_inner_color {0.48f, 0.75f, 0.95f, 1.f};
    float selection_border_thickness = 3.f;
    static constexpr size_t MaxSelections = 32;
    static constexpr size_t SelectionsVertexSize = sizeof(VertexPosColorTexcoord) * MaxSelections * 5 * 4;
    static constexpr size_t SelectionsIndexSize = sizeof(VertexPosColorTexcoord) * MaxSelections * 5 * 6;
    SelectionArea selections[MaxSelections];
    size_t selection_count = 0;
    ComPtr<ID3D11Buffer> selection_vertex_buffer;
    ComPtr<ID3D11Buffer> selection_index_buffer;

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
    ComPtr<ID3D11Texture2D> depth_stencil_texture;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view;

    /**/

    DirectX::XMFLOAT4 clearColor;
    HWND hwnd;
};

#endif // !GFX_HELPERS_H
