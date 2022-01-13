#include "Input.h"

void Mouse::UpdatePos(int x, int y)
{
    lastPos[0] = pos[0];
    lastPos[1] = pos[1];
    pos[0] = (float)x;
    pos[1] = (float)y;
    deltaMove[0] = pos[0] - lastPos[0];
    deltaMove[1] = pos[1] - lastPos[1];
}

void Mouse::UpdateWheelDelta(int dw)
{
    wheelDelta = dw;
}

void Mouse::Reset()
{
}

void Mouse::LockToBounds(float width, float height)
{

}

