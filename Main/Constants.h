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

enum TErrno{ OK, SUBBOARD, LOW_BATTARY, POWER_OFF, HIGH_WORKLOAD, TOO_SMALL_HEIGHT, LEG_CANNOT_REACH_POINT};
