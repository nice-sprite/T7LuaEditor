//
// Created by coxtr on 11/25/2021.
//

#ifndef T7LUAEDITOR_CAMERA_H
#define T7LUAEDITOR_CAMERA_H
#include <DirectXMath.h>
using namespace DirectX;
class Camera
{
public:
    Camera(float fov_radians, float aspect_ratio, float near_plane, float far_plane);
    void update_view(float timestep, XMMATRIX view);
    void update_projection(float timestep, XMMATRIX projection);
    void translate(XMFLOAT3 translation);
    void translate_from_vector(XMVECTOR translation);

    XMMATRIX get_transform();
    void set_aspect_ratio(float aspect_ratio);
    XMMATRIX const& get_projection() const;
    XMMATRIX const& get_view() const;

    // control functions 
    // might implement a CameraController thing that deals with animating the Camera
    // and sensitivity, panning, zoom, etc
    void zoom(int delta);
    void pan(float x, float y);
private:
    XMMATRIX view_matrix, projection_matrix; // can be orthographic or perspective
    
    XMVECTOR origin;

    float last_x{}, last_y{};
    float zoom_amount;
    float last_zoom{0};
    float zoom_speed_scale{0.5};
    float zNear, zFar, fov, aspect_ratio;
};

#endif //T7LUAEDITOR_CAMERA_H
