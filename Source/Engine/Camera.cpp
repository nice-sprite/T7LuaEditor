//
// Created by coxtr on 11/25/2021.
//

#include "Camera.h"
#include <imgui.h>
#include "win32_input.h"


Camera::Camera(float fovRadians, float aspectRatio, float nearPlane, float farPlane) :
    zNear(nearPlane), zFar(farPlane), fov(fovRadians)
{
    viewMat = DirectX::XMMatrixLookAtLH(
        DirectX::XMVECTOR{0.0, 0.0f, -800.f, 0.f}, // camera origin
        DirectX::XMVECTOR{0.0f, 0.0f, 0.0f, 0.f}, // focus point
        DirectX::XMVECTOR{0.f, 1.f, 0.f, 0.f}    // up direction
    );

    projectionMat = DirectX::XMMatrixPerspectiveFovLH(
        fovRadians,
        aspectRatio, 
        zNear, 
        zFar
    );

    input::register_callback([this](input::MouseState const& mouse, input::KeyboardState const& kbd) -> bool {
        if (kbd.ctrl_down) {
            zoom((int)(mouse.scroll_delta * -zoom_speed_scale));
        }

        if(kbd.ctrl_down && mouse.middle_down) {
            pan(mouse.x, mouse.y);
        } else {
            last_x = mouse.x;
            last_y = mouse.y;
        }
        return true;
    });

}

// control functions 
void Camera::zoom(int delta) {
    // dolly is moving the origin closer or further 
    // zoom is varying the FOV
    // another approach is to scale the world matrix
    translate_from_vector(DirectX::XMVectorSet(0.0, 0.0, (float)delta * zoom_speed_scale, 0.0));
}

void Camera::pan(float x, float y) {
    DirectX::XMVECTOR delta = DirectX::XMVectorSet(x-last_x, -(y - last_y), 0, 0);
    translate_from_vector(
        DirectX::XMVectorAdd(origin, delta)
    );
    last_x = x;
    last_y = y;
}

void Camera::update_view(float timestep, DirectX::XMMATRIX view)
{
    viewMat = view;
}

void Camera::update_projection(float timestep, DirectX::XMMATRIX projection)
{
    projectionMat = projection;
}

DirectX::XMMATRIX Camera::get_transform()
{
    return viewMat * projectionMat;
}

void Camera::set_aspect_ratio(float aspectRatio)
{
    projectionMat = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, zNear, zFar);
}

void Camera::translate_from_vector(DirectX::XMVECTOR translation) {

    viewMat *= DirectX::XMMatrixTranslationFromVector(translation);

}

void Camera::translate(DirectX::XMFLOAT3 translation)
{
    viewMat *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&translation));
}


DirectX::XMMATRIX& Camera::get_projection()
{
    return projectionMat;
}

DirectX::XMMATRIX& Camera::get_view()
{
    return viewMat;
}
