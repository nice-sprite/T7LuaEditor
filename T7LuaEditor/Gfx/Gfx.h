#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H

// d3d headers for creating d3d context to render with
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler

#include <imgui.h> // menu creation
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <fmt/format.h> // for error printing

// used to safely free Com Objects (ex. d3d context)
template <class T>
void SafeRelease( T** ppT )
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
};

namespace Gfx
{
    extern ID3D11Device* g_pDevice;
    extern IDXGISwapChain* g_pSwapChain;
    extern ID3D11DeviceContext* g_pContext;
    extern ID3D11RenderTargetView* g_pRenderTargetView ;
    extern ID3D11VertexShader* g_pVertexShader ;
    extern ID3D11InputLayout* g_pVertexLayout ;
    extern ID3D11PixelShader* g_pPixelShader ;
    extern ID3D11Buffer* g_pVertexBuffer ;
    extern ID3D11Buffer* g_pIndexBuffer ;
    extern ID3D11Buffer* g_pConstantBuffer;
    extern D3D11_VIEWPORT g_pViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    extern DirectX::XMMATRIX mOrtho;
    extern HWND g_hwnd;

#pragma region DefaultShader
    constexpr const char* szShadez = R"(
        // Constant buffer
        cbuffer ConstantBuffer : register(b0)
        {
	        matrix projection;
        }
        // PSI (PixelShaderInput)
        struct PSI
        {
	        float4 pos : SV_POSITION;
	        float4 color : COLOR;
        };
        // VertexShader
        PSI VS( float4 pos : POSITION, float4 color : COLOR )
        {
	        PSI psi;
	        psi.color = color;
	        pos = mul( pos, projection );
	        psi.pos = pos;
	        return psi;
        }
        // PixelShader
        float4 PS(PSI psi) : SV_TARGET
        {
	        return psi.color;
        }
        )";
#pragma endregion

    /*create a d3d context to render with*/
    bool StartDx11( HWND hwnd );
    void PrepareImGui();
    // free system resources when program exits
    void ShutdownDx11();

    bool CompileShader_Mem( const char* szShader,
        const char* szEntrypoint,
        const char* szTarget,
        ID3D10Blob** pBlob );

    void CreateRenderTarget();
    void CleanupRenderTarget();

    void Render();
    void Resize();

}
#endif // !GFX_HELPERS_H
