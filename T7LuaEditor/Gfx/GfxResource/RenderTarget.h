//
// Created by coxtr on 11/23/2021.
//

#ifndef T7LUAEDITOR_RENDERTARGET_H
#define T7LUAEDITOR_RENDERTARGET_H
#include "Bindable.h"

class RenderTarget : public Bindable{
    RenderTarget(Renderer& gfx);
    void Bind(Renderer& gfx);
};


#endif //T7LUAEDITOR_RENDERTARGET_H
