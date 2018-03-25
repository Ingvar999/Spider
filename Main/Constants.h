#pragma once

#ifndef CONSTANTS_H
#define CONSTANTS_H

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
#define minRadius 20
#define IntervalG 150
#define IntervalB 500
#define Lifting 30
#define SerialX Serial1
#define MaxSkew 5.0
#define dS 20

#endif
