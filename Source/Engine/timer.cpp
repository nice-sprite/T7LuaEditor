#include "timer.h"
#include "../defines.h"
#include <Windows.h>


Timer::Timer() {
  a = 0;
  b = 0;
  elapsed = 0;
  QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);
  seconds_per_count = 1.0 / (double)frequency;
}

double Timer::elapsed_ms() { return elapsed; }

void Timer::tick() {
  __int64 current_time;
  if (paused > 0) {
    elapsed = 0;
    return;
  }
  QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
  b = current_time;
  elapsed = (b - a) * seconds_per_count;
  a = b;
  if (elapsed < 0.0)
    elapsed = 0.0;
}

void Timer::start() { paused = 0; }

void Timer::stop() { paused = 1; }

void Timer::reset() {
  a = 0;
  b = 0;
  elapsed = 0;
}
