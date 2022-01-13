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
        DirectX::XMVECTOR{0.f, -1.f, 0.f, 0.f});

    projectionMat = DirectX::XMMatrixPerspectiveFovLH(
        fovRadians,
        aspectRatio, zNear, zFar);
}

void Camera::UpdateView(float timestep, DirectX::XMMATRIX view)
{
    viewMat = view;
}

void Camera::UpdateProjection(float timestep, DirectX::XMMATRIX projection)
{
    projectionMat = projection;
}

DirectX::XMMATRIX Camera::GetCameraTransform()
{
    return viewMat * projectionMat;
}

void Camera::SetAspectRatio(float aspectRatio)
{
    projectionMat = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, zNear, zFar);
}

void Camera::Translate(XMFLOAT3 translation)
{
    viewMat *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&translation));
}