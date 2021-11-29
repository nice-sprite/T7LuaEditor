//
// Created by coxtr on 11/25/2021.
//

#ifndef T7LUAEDITOR_DRAWME_H
#define T7LUAEDITOR_DRAWME_H
#include "GfxResource/VertexShader.h"
#include "GfxResource/VertexBuffer.h"
#include "GfxResource/InputLayout.h"
#include "GfxResource/IndexBuffer.h"
#include "GfxResource/PixelShader.h"
#include "GfxResource/ConstantBuffer.h"
#include "GfxResource/GraphicsResource.h"

class DrawMe : public GraphicsResource {
public:
    DrawMe(Gfx& gfx);
    void Draw(Gfx& gfx);

private:
//    VertexShader vertexShader;
//    InputLayout inputLayout;
//    VertexBuffer vertexBuffer;
//    IndexBuffer indexBuffer;
//    PixelShader pixelShader;


};


#endif //T7LUAEDITOR_DRAWME_H
