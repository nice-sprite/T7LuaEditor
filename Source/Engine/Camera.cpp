//
// Created by coxtr on 11/25/2021.
//

#include "Camera.h"

Camera::Camera(float fovRadians, float aspectRatio, float nearPlane, float farPlane) :
    zNear(nearPlane), zFar(farPlane), fov(fovRadians)
{
    viewMat = DirectX::XMMatrixLookAtLH(
        DirectX::XMVECTOR{0.0, 0.0f, 800.f, 0.f},
        DirectX::XMVECTOR{0.0f, 0.0f, 0.0f, 0.f},
        DirectX::XMVECTOR{0.f, -1.f, 0.f, 0.f}
    );

    projectionMat = DirectX::XMMatrixPerspectiveFovLH(
        fovRadians,
        aspectRatio, 
        zNear, 
        zFar
    );
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
