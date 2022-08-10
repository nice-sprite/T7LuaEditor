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
    void update_view(float timestep, DirectX::XMMATRIX view);
    void update_projection(float timestep, DirectX::XMMATRIX projection);
    void translate(DirectX::XMFLOAT3 translation);
    DirectX::XMMATRIX get_transform();
    void set_aspect_ratio(float aspectRatio);
    DirectX::XMMATRIX& get_projection();
    DirectX::XMMATRIX& get_view();

private:
    DirectX::XMMATRIX viewMat,
        projectionMat; // can be orthographic or perspective
    float zNear, zFar, fov, aspectRatio;
};

#endif //T7LUAEDITOR_CAMERA_H
