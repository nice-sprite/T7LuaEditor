//
// Created by coxtr on 11/25/2021.
//

#include "Camera.h"

Camera::Camera(Renderer &gfx) :
    x{}, y{}, z{}, pitch{}, roll{}, yaw{}, viewMatrix{}
{
    DirectX::XMFLOAT3 up, position, lookAt;
    DirectX::XMVECTOR upVec, positionVec, lookAtVec;

// TODO pickup here

}

void Camera::BindTo(Renderer &gfx) {

}

void Camera::SetTransform(Renderer &gfx) {

}
