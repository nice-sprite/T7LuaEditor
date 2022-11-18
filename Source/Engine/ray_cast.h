#pragma once

#include "../defines.h"
#include <DirectXMath.h>
#include <fmt/format.h>

struct Camera;
namespace ray_cast {

    using namespace DirectX;

    struct Ray {
        XMVECTOR origin;
        XMVECTOR direction;
    };


    XMFLOAT2 world_to_screen(XMFLOAT3 world, f32 width, f32 height, const Camera& camera);


    // converts screen coordinates to ray origin and direction
    Ray screen_to_world_ray(float x,
                            float y,
                            float width,
                            float height,
                            const Camera &camera,
                            XMMATRIX world
    );

    bool against_quad(
            Ray const &ray,
            float left,
            float right,
            float top,
            float bottom
    );

    inline XMVECTOR plane_from_quad(
            float left,
            float right,
            float top,
            float bottom);


    bool against_quad(
            Ray const &ray,
            XMFLOAT4 const &bounds
    );

    bool volume_intersection(Ray mins, Ray maxs, XMFLOAT4 quad);


}

template <> struct fmt::formatter<ray_cast::Ray> {
  constexpr auto parse(fmt::format_parse_context &ctx)
      -> decltype(ctx.begin()) {
    return ctx.end();
  }

  template <typename FormatContext>
  auto format(const ray_cast::Ray& ray, FormatContext &ctx) const
      -> decltype(ctx.out()) { 
    return fmt::format_to(ctx.out(), "origin: {}\ndirection: {}", ray.origin, ray.direction);

  }
};

