#ifndef INPUT_H
#define INPUT_H
#include <Windows.h>

class Mouse
{

public:
    Mouse() = default;

    void UpdatePos(int x, int y);
    void UpdateWheelDelta(int dw);
    void Reset();
    void LockToBounds(float width, float height);
    

public:
    float pos[2];
    float lastPos[2];
    float deltaMove[2];
    float bounds[2];

    int wheelDelta;
    int lastWheelDelta;

public:
    struct ButtonStates
    {
        bool leftButton,
            rightButton,
            scrollButton,
            xbutton1, // "forward" in browser
            xbutton2, // "backward" in browser
            scrollPlus,
            scrollMinus;
    };
    ButtonStates buttons;
};

class Keyboard
{
};
#endif