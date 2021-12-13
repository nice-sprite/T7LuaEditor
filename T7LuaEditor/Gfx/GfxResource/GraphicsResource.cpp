//
// Created by coxtr on 11/22/2021.
//

#include "GraphicsResource.h"

ID3D11DeviceContext *GraphicsResource::GetContext(Renderer &gfx) noexcept {
    return gfx.context.Get();
}


ID3D11Device *GraphicsResource::GetDevice(Renderer &gfx) noexcept {
    return gfx.device.Get();
}