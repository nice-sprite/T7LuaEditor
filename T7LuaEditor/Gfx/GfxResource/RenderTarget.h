//
// Created by coxtr on 11/23/2021.
//

#ifndef T7LUAEDITOR_RENDERTARGET_H
#define T7LUAEDITOR_RENDERTARGET_H
#include "Bindable.h"

class RenderTarget : public Bindable{
    RenderTarget(Gfx& gfx);
    void Bind(Gfx& gfx);
};


#endif //T7LUAEDITOR_RENDERTARGET_H
