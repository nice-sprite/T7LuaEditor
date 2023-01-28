#pragma once
#include "../defines.h"
#include "math.h"
#include "renderer_types.h"
#include <fmt/format.h>


struct Ray {
  Vec4 origin;
  Vec4 direction;
};

struct RayCaster {
  //CameraSystem *cam_sys;
  ViewportRegion active_viewport;

  //void init(CameraSystem *cam_sys);
  RayCaster() = default;
  RayCaster(RayCaster &) = delete;
  static RayCaster &instance();

  void set_viewport(ViewportRegion vp);
  Vec4 unproject(Vec4 screen);
  Ray picking_ray(Vec4 screen);

  Vec4 project(Vec4 world);

  // helpers

  // get the plane the quad is embedded in
  inline Vec4 quad_plane(Vec4 quad);

  // intersection tests
  bool ray_quad(Ray ray, Float4 quad_bounds);
  bool ray_volume(Ray min, Ray max, Float4 quad_bounds);
};

//Float2 world_to_screen(Float3 world, f32 width, f32 height, const Camera &camera);

// converts screen coordinates to ray origin and direction
//Ray screen_to_world_ray(float x,
//                        float y,
//                        float width,
//                        float height,
//                        const Camera &camera,
//                        Matrix world);
//
//bool against_quad(Ray const &ray,
//                  float left,
//                  float right,
//                  float top,
//                  float bottom);
//
inline Vec4 plane_from_quad(float left, float right, float top, float bottom);

bool against_quad(Ray const &ray, Float4 const &bounds);

bool volume_intersection(Ray mins, Ray maxs, Float4 quad);

template <> struct fmt::formatter<Ray> {
  constexpr auto parse(fmt::format_parse_context &ctx)
      -> decltype(ctx.begin()) {
    return ctx.end();
  }

  template <typename FormatContext>
  auto format(const Ray &ray, FormatContext &ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(),
                          "origin: {}\ndirection: {}",
                          ray.origin,
                          ray.direction);
  }
};
