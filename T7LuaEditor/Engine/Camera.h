//
// Created by coxtr on 11/25/2021.
//

#ifndef T7LUAEDITOR_CAMERA_H
#define T7LUAEDITOR_CAMERA_H
#include "GfxResource/Bindable.h"
#include <DirectXMath.h>
class Camera : public Bindable {
public:
    Camera(Renderer& gfx);
    void BindTo(Renderer& gfx);

    void SetTransform(Renderer& gfx);
private:
    float x, y, z, pitch, yaw, roll; // store the camera
    DirectX::XMMATRIX viewMatrix;
};


#endif //T7LUAEDITOR_CAMERA_H
