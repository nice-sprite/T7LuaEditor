#include "camera_system.h"
#include "renderer.h"
#include <imgui.h>

void CameraSystem::init() {
  register_cam();
  set_active(0);
}

// TODO this should probably not need to know about Renderer,
// only serve as a place to handle input to cameras, modes, etc
// So i think there is a better way to do this
void CameraSystem::update(Renderer &renderer, f32 timestep) {

  // map the constant buffer and update it
  renderer.update_shader_constants([=](PerSceneConsts &shader_constants) {
    shader_constants.modelViewProjection =
        XMMatrixIdentity() * get_active().get_transform();
  });

  if (ImGui::Begin("CameraSystem")) {
    ImGui::Text("active cam: %d ", active_cam);
    if (ImGui::Button("prev")) {
      set_active((active_cam - 1) % MaxCams);
    }
    ImGui::SameLine();
    if (ImGui::Button("next")) {
      set_active((active_cam + 1) % MaxCams);
    }
    ImGui::SliderAngle("fov", &get_active().fov, 25.0f, 120.0f);
    ImGui::Text("fov %f", get_active().fov);
  }
  ImGui::End();
}

// returns the index of the new camera
u32 CameraSystem::register_cam() {
  if (num_cams < MaxCams) {
    cameras[num_cams++] = Camera();
  }
  return num_cams;
}

// makes the camera at `index` the active camera
// if index refers to a valid camera
u32 CameraSystem::set_active(u32 index) {
  if (index >= 0 && index < MaxCams) {
    active_cam = index;
  }
  return active_cam;
}

// get a reference to the camera at `index`
Camera &CameraSystem::get_camera(u32 index) { return cameras[index]; }

// returns a reference to the currently active camera
Camera &CameraSystem::get_active() { return cameras[active_cam]; }
