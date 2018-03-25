#pragma once

#ifndef GYRO_H
#define GYRO_H

void CalibrationGyro();
void UpdateGyro();
inline int CheckGyro();
extern float angle_z, horizontal, vertical;

inline void Delay(unsigned int ms)
{
  unsigned long t = millis();
  while (millis() - t < ms)
    CheckGyro();
}

#endif 

