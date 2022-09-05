#pragma once
#include <DirectXMath.h>
#include "camera.h"
namespace ray_cast {

    using namespace DirectX;

    // converts screen coordinates to ray origin and direction
    void screen_to_world_ray(float x,
		 float y,
		 float width,
		 float height,
		 const Camera& camera,
		 XMMATRIX world,
         XMVECTOR& ray_origin,
         XMVECTOR& ray_dir
    );

    bool against_quad(
        XMVECTOR const& ray_origin,
        XMVECTOR const& ray_dir,
        float left,
        float right,
        float top,
        float bottom
    );

}


