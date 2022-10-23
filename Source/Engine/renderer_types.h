#include <DirectXMath.h>
using namespace DirectX;

struct VertexPosColorTexcoord
{
    XMFLOAT3 pos;
    XMFLOAT4 color;
    XMFLOAT2 texcoord;
};

struct DebugLine {
    XMFLOAT4 begin, end;
};

__declspec(align(16))
struct PerSceneConsts
{
    XMMATRIX modelViewProjection; // 64 bytes
    XMFLOAT4 timeTickDeltaFrame; // 16 bytes
    XMFLOAT2 viewportSize; // 8 bytes
    XMFLOAT2 windowSize; // 8 bytes
};
