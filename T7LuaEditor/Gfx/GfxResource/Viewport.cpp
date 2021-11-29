//
// Created by coxtr on 11/23/2021.
//

#include "Viewport.h"

Viewport::Viewport(Gfx &gfx) {
    width = gfx.GetWidth();
    height = gfx.GetHeight();
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;
    viewport.Width = (float)height;
    viewport.Height = (float)height;
    viewport.MaxDepth = 1.0f;
    viewport.MinDepth = 0.0f;
}

void Viewport::Bind(Gfx &gfx) {
    GetContext(gfx)->RSSetViewports(1, &viewport);
}
