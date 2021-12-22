#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H

#include <wrl/client.h> // Microsoft::WRL::ComPtr<T>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler
#pragma comment( lib, "d2d1.lib")
#pragma comment( lib, "dwrite.lib")
#pragma comment( lib, "dxguid.lib")

#include <imgui.h> // menu creation
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <fmt/format.h> // for error printing
#include <vector>
#include "Timer.h"
#include "Texture.h"
#include "ImGUIManager.h"
/*
#include "GfxResource/VertexShader.h"
#include "GfxResource/PixelShader.h"
*/

class VertexShader;
class PixelShader;
#include "Vertex.h"

namespace wrl = Microsoft::WRL;

class Renderer {
    friend class GraphicsResource; // this is where it started to stink...

public:
    /// <summary>
    /// Create a new rendering ctx_ for the window
    /// </summary>
    explicit Renderer(HWND hwnd, size_t _width, size_t _height);

    Renderer(const Renderer &Gfx) = delete;

    Renderer &operator=(const Renderer &) = delete;

    ~Renderer();

    void Render();

    void Resize(LPARAM lParam, WPARAM wParam);

    DirectX::XMMATRIX &GetOrthoMatrix() { return orthoMat_; }

    DirectX::XMMATRIX &GetProjectionMatrix() { return projectionMat_; }

    DirectX::XMMATRIX &GetWorldMatrix() { return worldMat_; }

    [[nodiscard]] size_t GetWidth() const { return width; }

    [[nodiscard]] size_t GetHeight() const { return height; }

private:

    bool InitializeDx11();
    bool SetupD2D();

    void CreateRenderTarget();
    void GetRenderTargetFor2D();
    void ClearRenderTargetFor2D();

    void CleanupRenderTarget();

    void DrawTestImage();

    struct ConstantBuffer {
        DirectX::XMMATRIX modelViewProjection;
    };

private:
    // D3D
    D3D11_VIEWPORT viewport_{};
    wrl::ComPtr<ID3D11BlendState> blendState_;
    wrl::ComPtr<ID3D11Buffer> constantBuf_;
    wrl::ComPtr<ID3D11Device5> device_;
    wrl::ComPtr<ID3D11DeviceContext4> ctx_;
    wrl::ComPtr<ID3D11RasterizerState2> rasterizerState_;
    wrl::ComPtr<ID3D11RenderTargetView> renderTargetView_;
    wrl::ComPtr<IDXGISwapChain4> swapChain_;
    wrl::ComPtr<IDXGIFactory7> dxgiFactory_;


    // D2D
    wrl::ComPtr<ID2D1Device6> device2d_;
    wrl::ComPtr<ID2D1DeviceContext6> context2d_;
    wrl::ComPtr<ID2D1RenderTarget> renderTarget2d_;
    wrl::ComPtr<IDWriteFactory7> writeFactory_;
    wrl::ComPtr<IDWriteTextFormat> textFormat_;
    wrl::ComPtr<ID2D1SolidColorBrush> whiteBrush_;


    std::unique_ptr<ImGUIManager> imgui_;
    DirectX::XMMATRIX orthoMat_{};
    DirectX::XMMATRIX projectionMat_{};
    DirectX::XMMATRIX worldMat_{};
    DirectX::XMMATRIX viewMat_{};

    DirectX::XMFLOAT4 backgroundColor_;
    HWND hwnd;
    size_t width, height;
    Timer frameTimer;
    Texture demoTexture;
};

#endif // !GFX_HELPERS_H