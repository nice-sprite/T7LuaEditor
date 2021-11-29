//
// Created by coxtr on 11/22/2021.
//
#include "ShaderUtil.h"

bool CompileShader_Disk(const wchar_t *filepath,
                        const char *szEntrypoint,
                        const char *szTarget,
                        ID3D10Blob **pBlob) {
    ID3D10Blob *pErrorBlob = nullptr;

    auto hr = D3DCompileFromFile(filepath, nullptr, nullptr, szEntrypoint, szTarget, D3DCOMPILE_ENABLE_STRICTNESS, 0,
                                 pBlob, &pErrorBlob);
    if (FAILED(hr)) {
        if (pErrorBlob) {
            char szError[256]{0};
            memcpy(szError, pErrorBlob->GetBufferPointer(), pErrorBlob->GetBufferSize());
            MessageBoxA(nullptr, szError, "Error", MB_OK);
        }
        return false;
    }
    return true;
}

bool CompileShader_Mem(const char *szShader,
                       const char *szEntrypoint,
                       const char *szTarget,
                       ID3D10Blob **pBlob) {
    ID3D10Blob *pErrorBlob = nullptr;

    auto hr = D3DCompile(szShader, strlen(szShader), 0, nullptr, nullptr, szEntrypoint, szTarget,
                         D3DCOMPILE_ENABLE_STRICTNESS, 0, pBlob, &pErrorBlob);
    if (FAILED(hr)) {
        if (pErrorBlob) {
            char szError[256]{0};
            memcpy(szError, pErrorBlob->GetBufferPointer(), pErrorBlob->GetBufferSize());
            MessageBoxA(nullptr, szError, "Error", MB_OK);
        }
        return false;
    }
    return true;
}

