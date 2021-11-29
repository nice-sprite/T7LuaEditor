//
// Created by coxtr on 11/23/2021.
//

#ifndef T7LUAEDITOR_VIEWPORT_H
#define T7LUAEDITOR_VIEWPORT_H
#include "Bindable.h"

class Viewport : public Bindable {
public:
    explicit Viewport(Gfx& gfx);
    void Bind(Gfx& gfx) override;
private:
    D3D11_VIEWPORT viewport{};
    size_t width, height;
};


#endif //T7LUAEDITOR_VIEWPORT_H
