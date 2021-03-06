#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H

#include "../t7pch.h"
#include "GfxResource/Everything.h"
#include "GfxResource/VertexBuffer.h"
#include "ImGUIManager.h"
#include "Texture.h"
#include "Vertex.h"

class Renderer
{

public:

    explicit Renderer(HWND hwnd, float _width, float _height);

    Renderer(const Renderer &Gfx) = delete;

    Renderer &operator=(const Renderer &) = delete;

    ~Renderer();

    void ClearRTV();

    void Present();

    void Resize(LPARAM lParam, WPARAM wParam);


    [[nodiscard]] float GetWidth() const;

    [[nodiscard]] float GetHeight() const;

    [[nodiscard]] ID3D11Device5 *GetDevice() const noexcept;
    [[nodiscard]] ID3D11DeviceContext *GetContext() const noexcept;

private:

    bool Init();

    bool SetupD2D();

    void CreateRenderTarget();

    void CleanupRenderTarget();

private:
    std::array<Texture, 3> defaultTextures_;

    ComPtr<ID3D11InputLayout> quadVertLayout_;

    int currentQuadCount = 0;
    const int MaxQuads = 20000;

    // D3D
    D3D11_VIEWPORT viewport{};
    ComPtr<ID3D11BlendState> alphaBlendState;
    ComPtr<ID3D11Device5> device;
    ComPtr<ID3D11DeviceContext4> context;
    ComPtr<ID3D11RasterizerState2> rasterizerState_;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<IDXGISwapChain4> swapChain;
    ComPtr<IDXGIFactory7> dxgiFactory;
    ComPtr<ID3D11SamplerState> gridSS;

    // D2D
    ComPtr<ID2D1Device6> device2D;
    ComPtr<ID2D1DeviceContext6> context2D;
    ComPtr<ID2D1RenderTarget> rtv2D;
    ComPtr<IDWriteFactory7> DWriteFactory;
    ComPtr<IDWriteTextFormat> defaultDWTextFormat;
    ComPtr<ID2D1SolidColorBrush> solidWhiteBrush2D;

    // Default shaders



    XMFLOAT4 clearColor;
    HWND hwnd;
    float width, height;
    Timer frameTimer;
    Texture demoTexture;
};

#endif // !GFX_HELPERS_H
