#include <DirectXMath.h>

class Camera;

struct GameInputMouseState;

namespace camera_controller {

    void flycam_fps(float dt, Camera &camera);

    void dollycam(float dt, Camera &camera);

};

