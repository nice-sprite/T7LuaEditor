//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_SHADERUTIL_H
#define T7LUAEDITOR_SHADERUTIL_H
#include <d3dcompiler.h>
bool Shader_CompileFromDisk(const wchar_t *filepath,
                                     const char *szEntrypoint,
                                     const char *szTarget,
                                     ID3D10Blob **pBlob) ;

bool Shader_CompileInMem(const char *szShader,
                                    const char *szEntrypoint,
                                    const char *szTarget,
                                    ID3D10Blob **pBlob);
#endif //T7LUAEDITOR_SHADERUTIL_H
