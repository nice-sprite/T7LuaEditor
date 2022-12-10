#pragma once

struct Timer {
  __int64 frequency;
  __int64 a;
  __int64 b;
  __int64 paused;
  double seconds_per_count;
  double elapsed;

  Timer();

  double elapsed_ms();

  void tick();

  void start();

  void stop();

  void reset();
};
