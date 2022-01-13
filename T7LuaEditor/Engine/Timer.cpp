//
// Created by coxtr on 11/29/2021.
//

#include "Timer.h"

Timer::Timer()
{
    QueryPerformanceFrequency(&clockFrequency);
}

void Timer::Start()
{
    QueryPerformanceCounter(&start);
}

// returns the number of microseconds since Start()
__int64 Timer::Stop()
{
    QueryPerformanceCounter(&end);
    elapsed.QuadPart = end.QuadPart - start.QuadPart;
    elapsed.QuadPart *= 1000000;
    elapsed.QuadPart /= clockFrequency.QuadPart;
    return elapsed.QuadPart; // returns microseconds
}

float Timer::StopMS()
{
    auto microseconds = Stop();
    return microseconds * 0.001;
}
