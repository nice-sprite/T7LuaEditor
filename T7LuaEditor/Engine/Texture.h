//
// Created by coxtr on 12/12/2021.
//

#ifndef T7LUAEDITOR_TEXTURE_H
#define T7LUAEDITOR_TEXTURE_H

#include "../Common.h"
#include <cassert>
#include <memory>
#include <wincodec.h>

//-------------------------------------------------------------------------------------
// WIC Pixel Format Translation Data
//-------------------------------------------------------------------------------------
struct WICTranslate
{
    GUID wic;
    DXGI_FORMAT format;
};

static WICTranslate g_WICFormats[] =
    {
        {GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT},

        {GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT},
        {GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM},

        {GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM},
        {GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM}, // DXGI 1.1
        {GUID_WICPixelFormat32bppBGR, DXGI_FORMAT_B8G8R8X8_UNORM},  // DXGI 1.1

        {GUID_WICPixelFormat32bppRGBA1010102XR, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM}, // DXGI 1.1
        {GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT_R10G10B10A2_UNORM},
        {GUID_WICPixelFormat32bppRGBE, DXGI_FORMAT_R9G9B9E5_SHAREDEXP},

#ifdef DXGI_1_2_FORMATS

        {GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM},
        {GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT_B5G6R5_UNORM},

#endif // DXGI_1_2_FORMATS

        {GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT_R32_FLOAT},
        {GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT},
        {GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R16_UNORM},
        {GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM},

        {GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT_A8_UNORM},

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
        {GUID_WICPixelFormat96bppRGBFloat, DXGI_FORMAT_R32G32B32_FLOAT},
#endif
};

//-------------------------------------------------------------------------------------
// WIC Pixel Format nearest conversion table
//-------------------------------------------------------------------------------------

struct WICConvert
{
    GUID source;
    GUID target;
};

static WICConvert g_WICConvert[] =
    {
        // Note target GUID in this conversion table must be one of those directly supported formats (above).

        {GUID_WICPixelFormatBlackWhite, GUID_WICPixelFormat8bppGray}, // DXGI_FORMAT_R8_UNORM

        {GUID_WICPixelFormat1bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat2bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat4bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat8bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM

        {GUID_WICPixelFormat2bppGray, GUID_WICPixelFormat8bppGray}, // DXGI_FORMAT_R8_UNORM
        {GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray}, // DXGI_FORMAT_R8_UNORM

        {GUID_WICPixelFormat16bppGrayFixedPoint, GUID_WICPixelFormat16bppGrayHalf},  // DXGI_FORMAT_R16_FLOAT
        {GUID_WICPixelFormat32bppGrayFixedPoint, GUID_WICPixelFormat32bppGrayFloat}, // DXGI_FORMAT_R32_FLOAT

#ifdef DXGI_1_2_FORMATS

        {GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat16bppBGRA5551}, // DXGI_FORMAT_B5G5R5A1_UNORM

#else

        {GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat16bppBGRA5551, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat16bppBGR565, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM

#endif // DXGI_1_2_FORMATS

        {GUID_WICPixelFormat32bppBGR101010,
         GUID_WICPixelFormat32bppRGBA1010102}, // DXGI_FORMAT_R10G10B10A2_UNORM

        {GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat24bppRGB, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat32bppPBGRA, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat32bppPRGBA, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM

        {GUID_WICPixelFormat48bppRGB, GUID_WICPixelFormat64bppRGBA},   // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat48bppBGR, GUID_WICPixelFormat64bppRGBA},   // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat64bppBGRA, GUID_WICPixelFormat64bppRGBA},  // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat64bppPRGBA, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat64bppPBGRA, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM

        {GUID_WICPixelFormat48bppRGBFixedPoint,
         GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat48bppBGRFixedPoint,
         GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat64bppRGBAFixedPoint,
         GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat64bppBGRAFixedPoint,
         GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat64bppRGBFixedPoint,
         GUID_WICPixelFormat64bppRGBAHalf},                                  // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat64bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat48bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT

        {GUID_WICPixelFormat96bppRGBFixedPoint,
         GUID_WICPixelFormat128bppRGBAFloat}, // DXGI_FORMAT_R32G32B32A32_FLOAT
        {GUID_WICPixelFormat128bppPRGBAFloat,
         GUID_WICPixelFormat128bppRGBAFloat}, // DXGI_FORMAT_R32G32B32A32_FLOAT
        {GUID_WICPixelFormat128bppRGBFloat,
         GUID_WICPixelFormat128bppRGBAFloat}, // DXGI_FORMAT_R32G32B32A32_FLOAT
        {GUID_WICPixelFormat128bppRGBAFixedPoint,
         GUID_WICPixelFormat128bppRGBAFloat}, // DXGI_FORMAT_R32G32B32A32_FLOAT
        {GUID_WICPixelFormat128bppRGBFixedPoint,
         GUID_WICPixelFormat128bppRGBAFloat}, // DXGI_FORMAT_R32G32B32A32_FLOAT

        {GUID_WICPixelFormat32bppCMYK, GUID_WICPixelFormat32bppRGBA},      // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat64bppCMYK, GUID_WICPixelFormat64bppRGBA},      // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat40bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat80bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
        {GUID_WICPixelFormat32bppRGB, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat64bppRGB, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat64bppPRGBAHalf,
         GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
#endif

        // We don't support n-channel formats
};

//--------------------------------------------------------------------------------------
static IWICImagingFactory *_GetWIC();

//---------------------------------------------------------------------------------
static DXGI_FORMAT _WICToDXGI(const GUID &guid);

//---------------------------------------------------------------------------------
static size_t _WICBitsPerPixel(REFGUID targetGuid);

//---------------------------------------------------------------------------------
static HRESULT CreateTextureFromWIC(_In_ ID3D11Device *d3dDevice,
                                    _In_opt_ ID3D11DeviceContext *d3dContext,
                                    _In_ IWICBitmapFrameDecode *frame,
                                    _Out_opt_ ID3D11Resource **texture,
                                    _Out_opt_ ID3D11ShaderResourceView **textureView,
                                    _In_ size_t maxsize);

struct Texture
{
    wrl::ComPtr<ID3D11Resource> texture_;
    wrl::ComPtr<ID3D11ShaderResourceView> view_;
    float width_, height_;
};

Texture LoadTexture(ID3D11Device *device, ID3D11DeviceContext *context, const wchar_t *path);

#endif //T7LUAEDITOR_TEXTURE_H
