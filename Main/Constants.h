#pragma once 

inline long sqr(const long x)
{
  return x * x;
}

inline int inc(int i, int d)
{
  i += d;
  if (i >= 6){
    i -= 6;
  }
  return i;
}

int sign(int32_t value){
  return ((value >> 31) << 1) + 1;
}

#define Pi 3.1416
#define ToRad 0.01745
#define ToGrad 57.3
#define L1 70
#define L2 70

const long sqrL1L2 = sqr(L1 + L2);
