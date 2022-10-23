#pragma once
#include <DirectXMath.h>
#include "camera.h"
namespace ray_cast {

    using namespace DirectX;

    struct Ray {
        XMVECTOR origin;
        XMVECTOR direction;
    };

    // converts screen coordinates to ray origin and direction
    Ray screen_to_world_ray(float x,
		 float y,
		 float width,
		 float height,
		 const Camera& camera,
		 XMMATRIX world
    );

    bool against_quad(
        Ray const& ray,
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
        Ray const& ray,
        XMFLOAT4 const& bounds
    );

    bool volume_intersection(Ray mins, Ray maxs, XMFLOAT4 quad);
}


