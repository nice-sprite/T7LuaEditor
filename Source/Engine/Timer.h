//
// Created by coxtr on 11/29/2021.
//

#ifndef T7LUAEDITOR_TIMER_H
#define T7LUAEDITOR_TIMER_H
#include <Windows.h>
class Timer {
private:
    LARGE_INTEGER clockFrequency;
    LARGE_INTEGER start, end, elapsed;
public:
    Timer();
    void Start();
    __int64 Stop();
    float StopMS();
};


#endif //T7LUAEDITOR_TIMER_H
