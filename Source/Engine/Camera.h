//
// Created by coxtr on 11/25/2021.
//

#ifndef T7LUAEDITOR_CAMERA_H
#define T7LUAEDITOR_CAMERA_H
#include <DirectXMath.h>
class Camera
{
public:
    Camera(float fovRadians, float aspectRatio, float nearPlane, float farPlane);
    void UpdateView(float timestep, DirectX::XMMATRIX view);
    void UpdateProjection(float timestep, DirectX::XMMATRIX projection);
    void Translate(DirectX::XMFLOAT3 translation);
    DirectX::XMMATRIX GetCameraTransform();
    void SetAspectRatio(float aspectRatio);
    DirectX::XMMATRIX& GetProjection();
    DirectX::XMMATRIX& GetView();


private:
    DirectX::XMMATRIX viewMat,
        projectionMat; // can be orthographic or perspective
    float zNear, zFar, fov, aspectRatio;
};

#endif //T7LUAEDITOR_CAMERA_H
