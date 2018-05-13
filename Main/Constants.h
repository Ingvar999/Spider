#pragma once

#ifndef _Constants_h_
#define _Constants_h_

inline long sqr(const long x)
{
  return x * x;
}

inline int inc(int i, int d)
{
  return (i + d) % 6;
}

#define Pi 3.1416
#define ToRad 0.01745
#define ToGrad 57.3
#define L1 70
#define L2 70
#define SerialX Serial1

#endif
