#pragma once
#include "../defines.h"
#include "camera.h"
struct Renderer;
struct CameraSystem {
  static constexpr u32 MaxCams = 16u;
  Camera cameras[MaxCams];
  u32 num_cams = 0;
  u32 active_cam = 0;

  void init(Renderer &renderer);
  void update(Renderer &renderer, f32 timestep);

  // returns the index of the new camera
  u32 register_cam();

  // makes the camera at `index` the active camera
  // if index refers to a valid camera
  u32 set_active(u32 index);

  // get a reference to the camera at `index`
  Camera &get_camera(u32 index);

  // returns a reference to the currently active camera
  Camera &get_active();
};