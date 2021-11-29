//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_GRAPHICSRESOURCE_H
#define T7LUAEDITOR_GRAPHICSRESOURCE_H

#include "../Gfx.h"

class Gfx;

class GraphicsResource {

protected:
    static ID3D11DeviceContext *GetContext(Gfx &gfx) noexcept;
    static ID3D11Device *GetDevice(Gfx &gfx) noexcept;
};


#endif //T7LUAEDITOR_GRAPHICSRESOURCE_H
