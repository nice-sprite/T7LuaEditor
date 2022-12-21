#include "ray_cast.h"
#include "camera.h"
#include "camera_system.h"
#include "imgui_fmt.h"
#include <DirectXMath.h>
// #include <algorithm>
#include "logging.h"
#include <imgui.h>

bool point_inside_rect(XMVECTOR point, XMFLOAT4 rect) {
  return XMVectorGetX(point) > rect.x && XMVectorGetX(point) < rect.y &&
         XMVectorGetY(point) > rect.z && XMVectorGetY(point) < rect.w;
}

// can we make the raycaster a per-viewport thing
// since the data is all related
// camera needs to have the correct aspectratio and viewport size,
// ray caster needs the camera for the viewport to shoot rays at it,
// and each viewport should only have 1 scene being rendered to it
RayCaster &RayCaster::instance() {
  static RayCaster rc;
  return rc;
}

void RayCaster::init(CameraSystem *cam_sys) { this->cam_sys = cam_sys; }

// transforms a world-space vector into screenspace
Vec4 RayCaster::project(Vec4 world) {

  Vec4 ret = DirectX::XMVector3Project(world,
                                       active_viewport.x,
                                       active_viewport.y,
                                       active_viewport.w,
                                       active_viewport.h,
                                       0.0,
                                       1.0,
                                       cam_sys->get_active().get_projection(),
                                       cam_sys->get_active().get_view(),
                                       XMMatrixIdentity());
  return ret;
}

// transforms a screen-space vector (2d xy) into a normalized Ray
Ray RayCaster::picking_ray(Vec4 screen) {
  Ray ray;
  screen =
      XMVectorSubtract(screen,
                       XMVECTORF32{active_viewport.x, active_viewport.y, 0, 0});
  LOG_INFO("mouse pos: {}", screen);
  Vec4 screen_near = XMVectorSetZ(screen, 0.0);
  Vec4 screen_far = XMVectorSetZ(screen, 1.0);
  ray.origin = unproject(screen_near);
  Vec4 dest = unproject(screen_far);
  ray.direction = XMVector3Normalize(XMVectorSubtract(dest, ray.origin));
  return ray;
}

Vec4 RayCaster::unproject(Vec4 xyz) {
  return XMVector3Unproject(xyz,
                            active_viewport.x,
                            active_viewport.y,
                            active_viewport.w,
                            active_viewport.h,
                            0.0,
                            1.0,
                            cam_sys->get_active().get_projection(),
                            cam_sys->get_active().get_view(),
                            XMMatrixIdentity());
}

bool RayCaster::ray_quad(Ray ray, Float4 quad_bounds) {
  Vec4 quad = XMLoadFloat4(&quad_bounds);
  Vec4 intersects =
      XMPlaneIntersectLine(quad_plane(quad),
                           ray.origin,
                           XMVectorAdd(ray.origin, ray.direction));

  return !XMVectorGetIntX(XMVectorIsNaN(intersects)) &&
         point_inside_rect(intersects, quad_bounds);
}

inline Vec4 RayCaster::quad_plane(Vec4 quad) {
  Float4 q;
  XMStoreFloat4(&q, quad);
  XMVECTOR left_top, left_bottom, right_bottom;
  left_top = XMVectorSet(q.x, q.z, 0.0, 0.0);
  left_bottom = XMVectorSet(q.x, q.w, 0.0, 0.0);
  right_bottom = XMVectorSet(q.y, q.w, 0.0, 0.0);
  return XMPlaneFromPoints(left_top, left_bottom, right_bottom);
}

void RayCaster::set_viewport(ViewportRegion vp) { active_viewport = vp; }
namespace ray_cast {

Ray screen_to_world_ray(float x,
                        float y,
                        float width,
                        float height,
                        const Camera &camera,
                        XMMATRIX world) {
  XMFLOAT4X4 projection;
  XMFLOAT4X4 inverse_view;
  XMVECTOR ray_origin, ray_dir;
  // XMVECTOR ray_origin, ray_dir;

  // load the matrices into addressible form
  XMStoreFloat4x4(&projection, camera.get_projection());
  XMStoreFloat4x4(&inverse_view, XMMatrixInverse(nullptr, camera.get_view()));

  ray_origin = XMVectorSet(0.0, 0.0, 0.0, 0.0);
  ray_dir = XMVectorSet((((2.0f * x) / width) - 1.0f) / projection(0, 0),
                        -(((2.0f * y) / height) - 1.0f) / projection(1, 1),
                        1.0,
                        0.0);

  XMMATRIX vi = XMMatrixInverse(nullptr, camera.get_view());
  ray_origin = XMVector3TransformCoord(ray_origin, vi);
  ray_dir = XMVector3Normalize(XMVector3TransformNormal(ray_dir, vi));
  return Ray{ray_origin, ray_dir};
}

inline XMVECTOR
plane_from_quad(float left, float right, float top, float bottom) {
  XMVECTOR left_top, left_bottom, right_bottom;
  left_top = XMVectorSet(left, top, 0.0, 0.0);
  left_bottom = XMVectorSet(left, bottom, 0.0, 0.0);
  right_bottom = XMVectorSet(right, bottom, 0.0, 0.0);

  return XMPlaneFromPoints(left_top, left_bottom, right_bottom);
}

// rect.x, rect.y == left, right
// rect.z, rect.w == top, bottom

// casts the given ray against a quad, returns true if the ray intersects the
// plane embedding the quad at a point inside the quad
bool against_quad(Ray const &ray,
                  float left,
                  float right,
                  float top,
                  float bottom) {

  XMVECTOR intersects;

  intersects = XMPlaneIntersectLine(plane_from_quad(left, right, top, bottom),
                                    ray.origin,
                                    XMVectorAdd(ray.origin, ray.direction));

  return !XMVectorGetIntX(XMVectorIsNaN(intersects)) &&
         point_inside_rect(intersects, XMFLOAT4{left, right, top, bottom});
}

// bool against_quad(Ray const &ray, XMFLOAT4 const &bounds) {
//   return against_quad(ray, bounds.x, bounds.y, bounds.z, bounds.w);
// }

bool volume_intersection(Ray mins, Ray maxs, XMFLOAT4 quad) {
  XMVECTOR mins_intersection; // where the mins ray intersects the quads plane
  XMVECTOR maxs_intersection; // where the maxs ray intersects the quads plane
  XMVECTOR quad_plane;        // the plane the quad is embedded in
  float left, right, top, bottom;

  left = quad.x;
  right = quad.y;
  top = quad.z;
  bottom = quad.w;

  quad_plane = plane_from_quad(quad.x, quad.y, quad.z, quad.w);

  mins_intersection =
      XMPlaneIntersectLine(quad_plane,
                           mins.origin,
                           XMVectorAdd(mins.origin, mins.direction));

  maxs_intersection =
      XMPlaneIntersectLine(quad_plane,
                           maxs.origin,
                           XMVectorAdd(maxs.origin, maxs.direction));

  // check for nan
  bool not_nan = !XMVectorGetIntX(XMVectorIsNaN(mins_intersection)) &&
                 !XMVectorGetIntX(XMVectorIsNaN(maxs_intersection));
  if (not_nan) {
    XMVECTOR right_bottom = XMVectorMax(mins_intersection, maxs_intersection);
    XMVECTOR top_left = XMVectorMin(mins_intersection, maxs_intersection);
    float l = XMVectorGetX(top_left);
    float r = XMVectorGetX(right_bottom);
    float t = XMVectorGetY(top_left);
    float b = XMVectorGetY(right_bottom);

    return ((l <= left && r >= left) || (l >= left && l <= right) ||
            (l <= left && r >= right)) &&
           ((t <= top && b >= top) || (t >= top && t <= bottom) ||
            (t <= top && b >= bottom));
  }
  return false;

  /*  ImGui::Separator();
    ImGui::Text("INTERSECTION DEBUG");
    ImGui::TextFmt("min_itx: {}\nmaxs_itx{}\n\n", mins_intersection,
                   maxs_intersection);
  */
}

// XMFLOAT2
// world_to_screen(XMFLOAT3 world, f32 width, f32 height, const Camera &camera)
// {
//   XMFLOAT2 ret;
//
//   XMVECTOR projected_coord = XMVector3Project(XMLoadFloat3(&world),
//                                               0.0f,
//                                               0.0f,
//                                               width,
//                                               height,
//                                               0.0f,
//                                               1.0f,
//                                               camera.get_projection(),
//                                               camera.get_view(),
//                                               XMMatrixIdentity());
//
//   XMStoreFloat2(&ret, projected_coord);
//   return ret;
// }

} // namespace ray_cast
