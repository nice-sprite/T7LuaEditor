#include "ray_cast.h"
#include "camera.h"
#include "imgui_fmt.h"
#include <DirectXMath.h>
#include <algorithm>
#include <imgui.h>

namespace ray_cast {

Ray screen_to_world_ray(float x, float y, float width, float height,
                        const Camera &camera, XMMATRIX world) {
  XMFLOAT4X4 projection;
  XMFLOAT4X4 inverse_view;
  XMVECTOR ray_origin, ray_dir;
  // XMVECTOR ray_origin, ray_dir;

  // load the matrices into addressible form
  XMStoreFloat4x4(&projection, camera.get_projection());
  XMStoreFloat4x4(&inverse_view, XMMatrixInverse(nullptr, camera.get_view()));

  ray_origin = XMVectorSet(0.0, 0.0, 0.0, 0.0);
  ray_dir =
      XMVectorSet((((2.0f * x) / width) - 1.0f) / projection(0, 0),
                  -(((2.0f * y) / height) - 1.0f) / projection(1, 1), 1.0, 0.0);

  XMMATRIX vi = XMMatrixInverse(nullptr, camera.get_view());
  ray_origin = XMVector3TransformCoord(ray_origin, vi);
  ray_dir = XMVector3Normalize(XMVector3TransformNormal(ray_dir, vi));
  return Ray{ray_origin, ray_dir};
}

inline XMVECTOR plane_from_quad(float left, float right, float top,
                                float bottom) {
  XMVECTOR left_top, left_bottom, right_bottom;
  left_top = XMVectorSet(left, top, 0.0, 0.0);
  left_bottom = XMVectorSet(left, bottom, 0.0, 0.0);
  right_bottom = XMVectorSet(right, bottom, 0.0, 0.0);

  return XMPlaneFromPoints(left_top, left_bottom, right_bottom);
}

// rect.x, rect.y == left, right
// rect.z, rect.w == top, bottom
bool point_inside_rect(XMVECTOR point, XMFLOAT4 rect) {
  return XMVectorGetX(point) > rect.x && XMVectorGetX(point) < rect.y &&
         XMVectorGetY(point) > rect.z && XMVectorGetY(point) < rect.w;
}

// casts the given ray against a quad, returns true if the ray intersects the
// plane embedding the quad at a point inside the quad
bool against_quad(Ray const &ray, float left, float right, float top,
                  float bottom) {

  XMVECTOR intersects;

  intersects =
      XMPlaneIntersectLine(plane_from_quad(left, right, top, bottom),
                           ray.origin, XMVectorAdd(ray.origin, ray.direction));

  return !XMVectorGetIntX(XMVectorIsNaN(intersects)) &&
         point_inside_rect(intersects, XMFLOAT4{left, right, top, bottom});
}

bool against_quad(Ray const &ray, XMFLOAT4 const &bounds) {
  return against_quad(ray, bounds.x, bounds.y, bounds.z, bounds.w);
}

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

  mins_intersection = XMPlaneIntersectLine(
      quad_plane, mins.origin, XMVectorAdd(mins.origin, mins.direction));

  maxs_intersection = XMPlaneIntersectLine(
      quad_plane, maxs.origin, XMVectorAdd(maxs.origin, maxs.direction));

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

XMFLOAT2 world_to_screen(XMFLOAT3 world, f32 width, f32 height,
                         const Camera &camera) {
  XMFLOAT2 ret;

  XMVECTOR projected_coord = XMVector3Project(
      XMLoadFloat3(&world), 0.0, 0.0, width, height, 0.0, 1.0,
      camera.get_projection(), camera.get_view(), XMMatrixIdentity());

  XMStoreFloat2(&ret, projected_coord);
  return ret;
}

} // namespace ray_cast
