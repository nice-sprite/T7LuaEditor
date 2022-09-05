#include "ray_cast.h"
namespace ray_cast {

    void screen_to_world_ray(float x,
        float y,
        float width,
        float height,
        const Camera& camera,
        XMMATRIX world,
        XMVECTOR& ray_origin,
        XMVECTOR& ray_dir
    ) {
 
        XMFLOAT4X4 projection;
        XMFLOAT4X4 inverse_view;
        //XMVECTOR ray_origin, ray_dir;

        // load the matrices into addressible form
        XMStoreFloat4x4(&projection, camera.get_projection());
        XMStoreFloat4x4(&inverse_view, XMMatrixInverse(nullptr, camera.get_view()));

        ray_origin = XMVectorSet(0.0, 0.0, 0.0, 0.0);
        ray_dir = XMVectorSet(
            (((2.0f * x) / width) - 1.0f) / projection(0, 0),
            -(((2.0f * y ) / height) - 1.0f) / projection(1, 1),
            1.0,
            0.0
        );

        XMMATRIX vi = XMMatrixInverse(nullptr, camera.get_view());
        ray_origin = XMVector3TransformCoord(ray_origin,  vi);
        ray_dir = XMVector3Normalize(XMVector3TransformNormal(ray_dir, vi));

    }

    inline XMVECTOR plane_from_quad(
        float left,
        float right,
        float top,
        float bottom
    ) {

        XMVECTOR left_top, left_bottom, right_bottom;
        left_top = XMVectorSet(left, top, 0.0, 0.0);
        left_bottom = XMVectorSet(left, bottom, 0.0, 0.0);
        right_bottom = XMVectorSet(right, bottom, 0.0, 0.0);

        return XMPlaneFromPoints(left_top, left_bottom, right_bottom);
    }

    // casts the given ray against a quad, returns true if the ray intersects the plane embedding the quad 
    // at a point inside the quad
    bool against_quad(
        XMVECTOR const& ray_origin,
        XMVECTOR const& ray_dir,
        float left,
        float right,
        float top,
        float bottom
    ) {

        XMVECTOR intersects;

        intersects = XMPlaneIntersectLine(
                plane_from_quad(left, right, top, bottom), 
                ray_origin, 
                XMVectorAdd(ray_origin, ray_dir)
                );

        return !XMVectorGetIntX(XMVectorIsNaN(intersects)) && 
            XMVectorGetX(intersects) > left && 
            XMVectorGetX(intersects) < right && 
            XMVectorGetY(intersects) > top && 
            XMVectorGetY(intersects) < bottom;
    }
}
