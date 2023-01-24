#include "math.h"

i32 clampi32(i32 min, i32 max, i32 val) {
  if(val < min) {
    val = min;
  } else if(val > max) {
    val = max;
  }
  return val;
}

f32 clampf32(f32 min, f32 max, f32 val) {
  if(val < min) {
    val = min;
  } else if(val > max) {
    val = max;
  }
  return val;
}

f64 clampf64(f64 min, f64 max, f64 val) {
  if(val < min) {
    val = min;
  } else if(val > max) {
    val = max;
  }
  return val;
}


i32 mini32(i32 a, i32 b) {
  if(a < b) return a;
  return b;
}

f32 minf32(f32 a, f32 b) {
  if(a < b) return a;
  return b;
}

f64 minf64(f64 a, f64 b) {
  if(a < b) return a;
  return b;
}
