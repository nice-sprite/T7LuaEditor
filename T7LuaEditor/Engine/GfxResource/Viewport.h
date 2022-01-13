//
// Created by coxtr on 11/23/2021.
//

#ifndef T7LUAEDITOR_VIEWPORT_H
#define T7LUAEDITOR_VIEWPORT_H
#include "../../t7pch.h"
class Viewport {
public:
    explicit Viewport(float x, float y, float width, float height);
    void Bind(ID3D11DeviceContext* context);

private:
    D3D11_VIEWPORT viewport{};
};


#endif //T7LUAEDITOR_VIEWPORT_H
