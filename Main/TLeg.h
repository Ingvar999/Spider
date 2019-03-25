#pragma once

#include <Servo.h>

class TLeg
{
  public:
    byte R;
    int workload;

    void Init(int, int, int, int, int, int, int);
    int ChangeHeight(int);
    int GetHeight() {
      return h;
    };
    int GetPosition() {
      return posOnBody;
    }
    int WriteAngles(int , int);
    void UpdateAngles(int& , int&);
    int CalculateForStep(int, int, int);
    inline int ReadVoltage();

  private:
    Servo servo1, servo2;
    int angle1, angle2;
    byte h = 0, measurePin, qR;
    short  p1, p2;
    int posOnBody;
};
