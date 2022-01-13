//
// Created by coxtr on 11/23/2021.
//

#include "Viewport.h"

Viewport::Viewport(float x, float y, float width, float height) {

    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MaxDepth = 1.0f;
    viewport.MinDepth = 0.0f;
}

void Viewport::Bind(ID3D11DeviceContext* context) {
    context->RSSetViewports(1, &viewport);
}
