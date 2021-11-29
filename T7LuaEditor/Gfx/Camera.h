//
// Created by coxtr on 11/25/2021.
//

#ifndef T7LUAEDITOR_CAMERA_H
#define T7LUAEDITOR_CAMERA_H
#include "GfxResource/Bindable.h"
#include <DirectXMath.h>
class Camera : public Bindable {
public:
    Camera(Gfx& gfx);
    void BindTo(Gfx& gfx);

    void SetTransform(Gfx& gfx);
private:
    float x, y, z, pitch, yaw, roll; // store the camera
    DirectX::XMMATRIX viewMatrix;
};


#endif //T7LUAEDITOR_CAMERA_H
