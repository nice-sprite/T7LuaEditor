//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_GRAPHICSRESOURCE_H
#define T7LUAEDITOR_GRAPHICSRESOURCE_H

#include "../Renderer.h"

class Renderer;

class GraphicsResource {

protected:
    static ID3D11DeviceContext *GetContext(Renderer &gfx) noexcept;
    static ID3D11Device *GetDevice(Renderer &gfx) noexcept;
};


#endif //T7LUAEDITOR_GRAPHICSRESOURCE_H
