#pragma once

#include "../defines.h"
#include <DirectXMath.h>

typedef DirectX::XMVECTOR Vec4;
typedef DirectX::XMFLOAT4 Float4;
typedef DirectX::XMFLOAT3 Float3;
typedef DirectX::XMFLOAT2 Float2;

typedef DirectX::XMMATRIX Matrix;
typedef DirectX::XMFLOAT4X4 MatrixF32;

typedef Float2 ScreenPos;

i32 clampi32(i32 min, i32 max, i32 val);
f32 clampf32(f32 min, f32 max, f32 val);
f64 clampf64(f64 min, f64 max, f64 val);


i32 mini32(i32 a, i32 b);
f32 minf32(f32 a, f32 b);
f64 minf64(f64 a, f64 b);
