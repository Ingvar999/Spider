#pragma once
#ifndef TLEG_H
#define TLEG_H

#include <Servo.h>

class TLeg
{
  public:
    byte R;
    void Init(int, int, int, int, int, int, int);
    int SetHeight(int);
    int GetHeight() {
      return h;
    };
    bool HasContact() {
      return digitalRead(contactPin);
    }
    int GetPosition() {
      return pos;
    }
    int WriteAngle(int , int);
    void UpdateAngle(int& , int&);
    int ForStep(int,int,int);

  private:
    Servo servo1, servo2;
    byte angle1, angle2, h = 0, contactPin, qR;
    short  p1, p2, pos;
};

#endif
