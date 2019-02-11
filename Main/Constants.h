#pragma once

inline long sqr(const long x)
{
  return x * x;
}

inline String BoolToString(bool b){return (b ? "1" : "0");}

inline int inc(int i, int d)
{
  return (i + d) % 6;
}

int sign(int32_t value){
  return ((value >> 31) << 1) + 1;
}

#define Pi 3.1416
#define ToRad 0.01745
#define ToGrad 57.3
#define L1 70
#define L2 70

#define OK 0
#define SUBBOARD 1
#define LOW_BATTARY 2
#define POWER_OFF 3
#define TOO_SMALL_HEIGHT 4
#define LEG_CANNOT_REACH_POINT 5
