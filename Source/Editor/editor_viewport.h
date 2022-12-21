#include "../Engine/ray_cast.h"
// #include "../Engine/input_system.h"

struct EditorViewport {

  RayCaster picking;

  i32 renderer_viewport_index; // the index of the RTV the renderer should bind
                               // for this view
};
