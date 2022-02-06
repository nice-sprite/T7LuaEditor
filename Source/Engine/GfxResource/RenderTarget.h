//
// Created by coxtr on 11/23/2021.
//

#ifndef T7LUAEDITOR_RENDERTARGET_H
#define T7LUAEDITOR_RENDERTARGET_H
#include <d3d11_4.h>
class RenderTarget {
    RenderTarget(ID3D11Device* device);
    void Bind(ID3D11DeviceContext* context);
};


#endif //T7LUAEDITOR_RENDERTARGET_H
