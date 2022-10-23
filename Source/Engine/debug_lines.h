#include "renderer_types.h"
#include "ray_cast.h"

constexpr unsigned int MaxDebugLines = 1024;
enum DebugColors: XMVECTOR  {
    Red   =   XMFLOAT4{1.0, 0.0, 0.0, 1.0},
    Green =   XMFLOAT4{0.0, 1.0, 0.0, 1.0},
    Blue  =   XMFLOAT4{0.0, 0.0, 1.0, 1.0},
    Pink  =   XMFLOAT4{0.5, 0.0, 1.0, 1.0}
};

struct DebugLineSystem {
    DebugLine lines[MaxDebugLines];
    unsigned int n_lines = 0;

    void add_line(XMVECTOR a, XMVECTOR b);
    void add_ray(ray_cast::Ray ray);

    void clear_debug_lines();

    void set_line(unsigned int i, XMFLOAT3 begin, XMFLOAT3 end, XMFLOAT4 color);
    void set_line_vector(unsigned int i, XMVECTOR begin, XMVECTOR end, XMFLOAT4 color);
    void set_line_color(unsigned int i, XMFLOAT4 color);

    void update();
};

