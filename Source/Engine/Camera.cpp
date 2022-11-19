//
// Created by coxtr on 11/25/2021.
//

#include "camera.h"
#include "win32_input.h"
#include <DirectXMath.h>
#include <imgui.h>

Camera::Camera(float fov_radians, float aspect_ratio, float near_plane,
               float far_plane) {
  zNear = near_plane;
  zFar = far_plane;
  fov = fov_radians;
  pitch = 0.0;
  yaw = 0.0;
  roll = 0.0;
  rotation = XMMatrixIdentity();
  up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
  origin = XMVectorSet(0.0, 0.0f, -800.f, 0.f);
  view_matrix =
      XMMatrixLookAtLH(origin, // camera origin
                       XMVECTOR{0.0f, 0.0f, 0.0f, 0.f},  // focus point
                       XMVectorSet(0.f, 1.f, 0.f, 0.f)      // up direction
      );

  projection_matrix =
      XMMatrixPerspectiveFovLH(fov_radians, aspect_ratio, zNear, zFar);
}

Camera::Camera() : Camera(1.39626, 16.0f / 9.0f, 1.0f, 9999.0f) {}

// control functions
void Camera::zoom(int delta) {
  // dolly is moving the origin closer or further
  // zoom is varying the FOV
  // another approach is to scale the world matrix
  // translate_from_vector(XMVectorSet(0.0, 0.0, (float)delta *
  // zoom_speed_scale, 0.0));
}

void Camera::pan(float x, float y) {
  // XMVECTOR delta = XMVectorSet(x-last_x, -(y - last_y), 0, 0);
  // translate_from_vector(
  //     XMVectorAdd(origin, delta)
  //);
  // last_x = x;
  // last_y = y;
}

void Camera::update_view(XMMATRIX view) { view_matrix = view; }

void Camera::update_projection(XMMATRIX projection) {
  projection_matrix = projection;
}

XMMATRIX Camera::get_transform() { return view_matrix * projection_matrix; }

void Camera::set_aspect_ratio(float aspect_ratio) {
  projection_matrix = XMMatrixPerspectiveFovLH(fov, aspect_ratio, zNear, zFar);
}

void Camera::translate_from_vector(XMVECTOR translation) {

  view_matrix *= XMMatrixTranslationFromVector(translation);
}

void Camera::translate(XMFLOAT3 translation) {
  view_matrix *= XMMatrixTranslationFromVector(XMLoadFloat3(&translation));
}

XMMATRIX const &Camera::get_projection() const { return projection_matrix; }

XMMATRIX const &Camera::get_view() const { return view_matrix; }
