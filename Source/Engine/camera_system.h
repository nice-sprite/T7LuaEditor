#pragma once
#include "../defines.h"
#include "math.h"

enum CameraMode {
  CAM_FLYCAM,
  CAM_DOLLY
};

struct Camera {
  CameraMode mode;
  float pitch, yaw, roll;
  // float last_x{}, last_y{};
 // float zoom_amount;
 // float last_zoom{0};
  //float zoom_speed_scale{0.5};
  float zNear, zFar, fov, aspect_ratio;

  Vec4 forward, up, right;
  Vec4 origin;
  Matrix rotation;
  // can be orthographic or perspective
  Matrix view_matrix;
  Matrix projection_matrix; 
};


struct CameraSystem {
  CameraSystem() = default;
  CameraSystem(CameraSystem &) = delete;
  CameraSystem &operator=(CameraSystem &) = delete;

  static constexpr u32 MaxCams = 16u;
  Camera cameras[MaxCams];
  u32 num_cams = 0;
  u32 active_cam = 0;

  /* camera movement settings */
  f32 normal_speed;
  f32 fast_speed;
  f32 slow_speed;

  f32 smoothness;
  f32 inertia;


//  void update(Renderer &renderer, f32 timestep);

  // returns the index of the new camera
  u32 register_cam();

  // makes the camera at `index` the active camera
  // if index refers to a valid camera
  u32 set_active(u32 index);

  // get a reference to the camera at `index`
  Camera *get_camera(u32 index);

  // returns a reference to the currently active camera
  Camera *get_active();
};

void camera_flycam(CameraSystem* cam_sys, struct InputSystem* input, f32 time_delta);
void camera_dolly(CameraSystem* cam_sys, struct InputSystem* input, f32 time_delta);

