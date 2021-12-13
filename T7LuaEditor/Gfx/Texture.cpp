//
// Created by coxtr on 12/12/2021.
//

#include "Texture.h"

DXGI_FORMAT _WICToDXGI(const GUID &guid) {
    for (size_t i = 0; i < _countof(g_WICFormats); ++i) {
        if (memcmp(&g_WICFormats[i].wic, &guid, sizeof(GUID)) == 0)
            return g_WICFormats[i].format;
    }

    return DXGI_FORMAT_UNKNOWN;
}

IWICImagingFactory *_GetWIC() {
    static IWICImagingFactory *s_Factory = nullptr;

    if (s_Factory)
        return s_Factory;

    HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            __uuidof(IWICImagingFactory),
            (LPVOID *) &s_Factory
    );

    if (FAILED(hr)) {
        s_Factory = nullptr;
        return nullptr;
    }

    return s_Factory;
}

size_t _WICBitsPerPixel(const GUID &targetGuid) {
    IWICImagingFactory *pWIC = _GetWIC();
    if (!pWIC)
        return 0;

    wrl::ComPtr<IWICComponentInfo> cinfo;
    if (FAILED(pWIC->CreateComponentInfo(targetGuid, &cinfo)))
        return 0;

    WICComponentType type;
    if (FAILED(cinfo->GetComponentType(&type)))
        return 0;

    if (type != WICPixelFormat)
        return 0;

    wrl::ComPtr<IWICPixelFormatInfo> pfinfo;
    if (FAILED(cinfo->QueryInterface(__uuidof(IWICPixelFormatInfo), (void **) (&pfinfo))))
        return 0;

    UINT bpp;
    if (FAILED(pfinfo->GetBitsPerPixel(&bpp)))
        return 0;

    return bpp;
}

HRESULT CreateTextureFromWIC(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext, IWICBitmapFrameDecode *frame,
                             ID3D11Resource **texture, ID3D11ShaderResourceView **textureView, size_t maxsize) {
    UINT width, height;
    HRESULT hr = frame->GetSize(&width, &height);
    if (FAILED(hr))
        return hr;

    assert(width > 0 && height > 0);

    if (!maxsize) {
        // This is a bit conservative because the hardware could support larger textures than
        // the Feature Level defined minimums, but doing it this way is much easier and more
        // performant for WIC than the 'fail and retry' model used by DDSTextureLoader

        switch (d3dDevice->GetFeatureLevel()) {
            case D3D_FEATURE_LEVEL_9_1:
            case D3D_FEATURE_LEVEL_9_2:
                maxsize = 2048 /*D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
                break;

            case D3D_FEATURE_LEVEL_9_3:
                maxsize = 4096 /*D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
                break;

            case D3D_FEATURE_LEVEL_10_0:
            case D3D_FEATURE_LEVEL_10_1:
                maxsize = 8192 /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
                break;

            default:
                maxsize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                break;
        }
    }

    assert(maxsize > 0);

    UINT twidth, theight;
    if (width > maxsize || height > maxsize) {
        float ar = static_cast<float>(height) / static_cast<float>(width);
        if (width > height) {
            twidth = static_cast<UINT>( maxsize );
            theight = static_cast<UINT>( static_cast<float>(maxsize) * ar );
        } else {
            theight = static_cast<UINT>( maxsize );
            twidth = static_cast<UINT>( static_cast<float>(maxsize) / ar );
        }
        assert(twidth <= maxsize && theight <= maxsize);
    } else {
        twidth = width;
        theight = height;
    }

    // Determine format
    WICPixelFormatGUID pixelFormat;
    hr = frame->GetPixelFormat(&pixelFormat);
    if (FAILED(hr))
        return hr;

    WICPixelFormatGUID convertGUID;
    memcpy(&convertGUID, &pixelFormat, sizeof(WICPixelFormatGUID));

    size_t bpp = 0;

    DXGI_FORMAT format = _WICToDXGI(pixelFormat);
    if (format == DXGI_FORMAT_UNKNOWN) {
        for (size_t i = 0; i < _countof(g_WICConvert); ++i) {
            if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0) {
                memcpy(&convertGUID, &g_WICConvert[i].target, sizeof(WICPixelFormatGUID));

                format = _WICToDXGI(g_WICConvert[i].target);
                assert(format != DXGI_FORMAT_UNKNOWN);
                bpp = _WICBitsPerPixel(convertGUID);
                break;
            }
        }

        if (format == DXGI_FORMAT_UNKNOWN)
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    } else {
        bpp = _WICBitsPerPixel(pixelFormat);
    }

    if (!bpp)
        return E_FAIL;

    // Verify our target format is supported by the current device
    // (handles WDDM 1.0 or WDDM 1.1 device driver cases as well as DirectX 11.0 Runtime without 16bpp format support)
    UINT support = 0;
    hr = d3dDevice->CheckFormatSupport(format, &support);
    if (FAILED(hr) || !(support & D3D11_FORMAT_SUPPORT_TEXTURE2D)) {
        // Fallback to RGBA 32-bit format which is supported by all devices
        memcpy(&convertGUID, &GUID_WICPixelFormat32bppRGBA, sizeof(WICPixelFormatGUID));
        format = DXGI_FORMAT_R8G8B8A8_UNORM;
        bpp = 32;
    }

    // Allocate temporary memory for image
    size_t rowPitch = (twidth * bpp + 7) / 8;
    size_t imageSize = rowPitch * theight;

    std::unique_ptr<uint8_t[]> temp(new uint8_t[imageSize]);

    // Load image data
    if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0
        && twidth == width
        && theight == height) {
        // No format conversion or resize needed
        hr = frame->CopyPixels(0, static_cast<UINT>( rowPitch ), static_cast<UINT>( imageSize ), temp.get());
        if (FAILED(hr))
            return hr;
    } else if (twidth != width || theight != height) {
        // Resize
        IWICImagingFactory *pWIC = _GetWIC();
        if (!pWIC)
            return E_NOINTERFACE;

        wrl::ComPtr<IWICBitmapScaler> scaler;
        hr = pWIC->CreateBitmapScaler(&scaler);
        if (FAILED(hr))
            return hr;

        hr = scaler->Initialize(frame, twidth, theight, WICBitmapInterpolationModeFant);
        if (FAILED(hr))
            return hr;

        WICPixelFormatGUID pfScaler;
        hr = scaler->GetPixelFormat(&pfScaler);
        if (FAILED(hr))
            return hr;

        if (memcmp(&convertGUID, &pfScaler, sizeof(GUID)) == 0) {
            // No format conversion needed
            hr = scaler->CopyPixels(0, static_cast<UINT>( rowPitch ), static_cast<UINT>( imageSize ), temp.get());
            if (FAILED(hr))
                return hr;
        } else {
            wrl::ComPtr<IWICFormatConverter> FC;
            hr = pWIC->CreateFormatConverter(&FC);
            if (FAILED(hr))
                return hr;

            hr = FC->Initialize(scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0,
                                WICBitmapPaletteTypeCustom);
            if (FAILED(hr))
                return hr;

            hr = FC->CopyPixels(0, static_cast<UINT>( rowPitch ), static_cast<UINT>( imageSize ), temp.get());
            if (FAILED(hr))
                return hr;
        }
    } else {
        // Format conversion but no resize
        IWICImagingFactory *pWIC = _GetWIC();
        if (!pWIC)
            return E_NOINTERFACE;

        wrl::ComPtr<IWICFormatConverter> FC;
        hr = pWIC->CreateFormatConverter(&FC);
        if (FAILED(hr))
            return hr;

        hr = FC->Initialize(frame, convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
            return hr;

        hr = FC->CopyPixels(0, static_cast<UINT>( rowPitch ), static_cast<UINT>( imageSize ), temp.get());
        if (FAILED(hr))
            return hr;
    }

    // See if format is supported for auto-gen mipmaps (varies by feature level)
    bool autogen = false;
    if (d3dContext != 0 && textureView != 0) // Must have context and shader-view to auto generate mipmaps
    {
        UINT fmtSupport = 0;
        hr = d3dDevice->CheckFormatSupport(format, &fmtSupport);
        if (SUCCEEDED(hr) && (fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN)) {
            autogen = true;
        }
    }

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = twidth;
    desc.Height = theight;
    desc.MipLevels = (autogen) ? 0 : 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = (autogen) ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : (D3D11_BIND_SHADER_RESOURCE);
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = (autogen) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = temp.get();
    initData.SysMemPitch = static_cast<UINT>( rowPitch );
    initData.SysMemSlicePitch = static_cast<UINT>( imageSize );

    ID3D11Texture2D *tex = nullptr;
    hr = d3dDevice->CreateTexture2D(&desc, (autogen) ? nullptr : &initData, &tex);
    if (SUCCEEDED(hr) && tex != 0) {
        if (textureView != 0) {
            D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
            memset(&SRVDesc, 0, sizeof(SRVDesc));
            SRVDesc.Format = format;
            SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = (autogen) ? -1 : 1;

            hr = d3dDevice->CreateShaderResourceView(tex, &SRVDesc, textureView);
            if (FAILED(hr)) {
                tex->Release();
                return hr;
            }

            if (autogen) {
                assert(d3dContext != 0);
                d3dContext->UpdateSubresource(tex, 0, nullptr, temp.get(), static_cast<UINT>(rowPitch),
                                              static_cast<UINT>(imageSize));
                d3dContext->GenerateMips(*textureView);
            }
        }

        if (texture != 0) {
            *texture = tex;
        } else {
#if defined(_DEBUG) || defined(PROFILE)
            tex->SetPrivateData(WKPDID_D3DDebugObjectName,
                                sizeof("WICTextureLoader") - 1,
                                "WICTextureLoader"
            );
#endif
            tex->Release();
        }
    }

    return hr;
}

Texture LoadTexture(ID3D11Device *device, ID3D11DeviceContext *context, const wchar_t *path) {

    wrl::ComPtr<IWICImagingFactory> WICFactory_;
    wrl::ComPtr<IWICBitmapDecoder> decoder_;
    wrl::ComPtr<IWICBitmapFrameDecode> frame_;
    HRESULT result = CoCreateInstance(CLSID_WICImagingFactory,
                                      nullptr,
                                      CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&WICFactory_));
    DX_ASSERT(result, "failed to create WICImagingFactory");

    result = WICFactory_->CreateDecoderFromFilename(path,
                                                    nullptr,
                                                    GENERIC_READ,
                                                    WICDecodeMetadataCacheOnDemand,
                                                    &decoder_);
    DX_ASSERT(result, "failed to create Decoder");

    result = decoder_->GetFrame(0, &frame_);
    DX_ASSERT(result, "failed to get frame from texture");

    Texture tex;
    CreateTextureFromWIC(device, context, frame_.Get(), &(tex.texture_), &(tex.view_), 0);
    return tex;

}
