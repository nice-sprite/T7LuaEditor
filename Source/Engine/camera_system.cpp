#include "camera_system.h"
#include "input_system.h"


// TODO this should probably not need to know about Renderer,
// only serve as a place to handle input to cameras, modes, etc
// So i think there is a better way to do this
//void CameraSystem::update(Renderer &renderer, f32 timestep) {
//  // map the constant buffer and update it
//  renderer.update_shader_constants([&](PerSceneConsts &shader_constants) {
//    shader_constants.modelViewProjection =
//        XMMatrixIdentity() * this->get_active()->get_transform();
//  });
//
//  if (ImGui::Begin("CameraSystem")) {
//    ImGui::Text("active cam: %d ", active_cam);
//    if (ImGui::Button("prev")) {
//      set_active((active_cam - 1) % MaxCams);
//    }
//    ImGui::SameLine();
//    if (ImGui::Button("next")) {
//      set_active((active_cam + 1) % MaxCams);
//    }
//    ImGui::SliderAngle("fov", &get_active()->fov, 25.0f, 120.0f);
//    ImGui::Text("fov %f", get_active()->fov);
//  }
//  ImGui::End();
//}
//

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
Camera *CameraSystem::get_camera(u32 index) { return &cameras[index]; }

// returns a reference to the currently active camera
Camera *CameraSystem::get_active() { return &cameras[active_cam]; }


void camera_flycam(CameraSystem* cam_sys, InputSystem* input, f32 time_delta) {

  Camera* camera = cam_sys->get_active();

  static XMVECTOR default_forward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
  static XMVECTOR default_right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
  XMVECTOR camera_target;
  XMVECTOR camera_offset = XMVectorSet(0, 0, 0, 0);
  XMMATRIX rotation;
  XMMATRIX temp_y_matrix;
  float movement_speed = 500.0f;

  if (input->mouse_buttons.left) {
    camera->pitch += 0.003 * (float)(input->mouse_delta.dy);
    camera->yaw += 0.003 * (float)(input->mouse_delta.dx);
    camera->pitch = clampf32(camera->pitch,
                               (float)(-XM_PIDIV2 + 0.001),
                               (float)(XM_PIDIV2 - 0.001));

    if (input->key_down('W')) {
      camera_offset = XMVectorAdd(camera_offset, XMVectorSet(0.0, 0.0, 1.0, 0.0));
    }

    if (input->key_down('S')) {
      camera_offset = XMVectorAdd(camera_offset, XMVectorSet(0.0, 0.0, -1.0, 0.0));
    }

    if (input->key_down('A')) {
      camera_offset = XMVectorAdd(camera_offset, XMVectorSet(-1.0, 0.0, 0.0, 0.0));
    }

    if (input->key_down('D')) {
      camera_offset = XMVectorAdd(camera_offset, XMVectorSet(1.0, 0.0, 0.0, 0.0));
    }
  }

  rotation = XMMatrixRotationRollPitchYaw(camera->pitch, camera->yaw, camera->roll);
  temp_y_matrix = XMMatrixRotationY(camera->yaw);

  if (input->key_down(VK_LSHIFT)) {
    movement_speed = 1500.0f;
  } else if (input->key_down(VK_LCONTROL)) {
    movement_speed = 100.0;
  }

  camera->origin += (f32)time_delta * movement_speed * XMVector3Normalize(XMVector3TransformCoord(camera_offset, rotation));

  if (input->key_down('E'))
    camera->origin += (f32)time_delta * movement_speed * XMVectorSet(0.0, 1.0, 0.0, 0.0);
  if (input->key_down('Q'))
    camera->origin += (f32)time_delta * movement_speed * XMVectorSet(0.0, -1.0, 0.0, 0.0);

  camera_target = XMVector3Normalize(XMVector3TransformCoord(default_forward, rotation));
  camera->forward = XMVector3TransformCoord(default_forward, temp_y_matrix);
  camera->up = XMVector3TransformCoord(camera->up, temp_y_matrix);
  camera->right = XMVector3TransformCoord(default_right, temp_y_matrix);
  camera->view_matrix = XMMatrixLookAtLH(camera->origin, camera->origin + camera_target, camera->up);
}

void camera_dolly(CameraSystem* cam_sys, struct InputSystem* input, f32 time_delta) {

  Camera* camera = cam_sys->get_active();
  static XMVECTOR default_forward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
  static XMVECTOR default_right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
  static float movement_speed = 10.0;
  XMMATRIX rotation;
  XMVECTOR camera_offset = XMVectorSet(0, 0, 0, 0);
  XMVECTOR camera_target;

  rotation = XMMatrixRotationRollPitchYaw(camera->pitch, camera->yaw, camera->roll);
  if (input->key_down(VK_LCONTROL)) {
    camera_offset += XMVectorSet(0.0, 0.0, movement_speed * time_delta * (float)(input->mouse_wheel), 0.0);
    if (input->mouse_buttons.left) {
      camera_offset += XMVectorSet(-(float)(input->mouse_delta.dx), (float)(input->mouse_delta.dy), 0.0, 0.0);
    }
  }
  camera->origin += XMVector3TransformCoord(camera_offset, rotation);
  camera_target = XMVector3Normalize(XMVector3TransformCoord(default_forward, rotation));
  camera->view_matrix = XMMatrixLookAtLH(camera->origin, camera->origin + camera_target, camera->up);
}
