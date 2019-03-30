#include "TLeg.h"
#include "Constants.h"

void TLeg::Init(int _posOnBody, int _measurePin, int pin1, int pin2, int pog1, int pog2, int _qR)
{
  posOnBody = _posOnBody;
  measurePin = _measurePin;
  qR = _qR;
  p1 = pog1;
  p2 = pog2;
  servo1.attach(pin1);
  servo2.attach(pin2);
}

int TLeg::ChangeHeight(int delta)
{
  int newh = h + delta;
  if ((newh >= 0) && (sqrL1L2 > (sqr(newh) + sqr(R))))
  {
    h = newh;
    return 0;
  }
  else
    return 1;
}

int TLeg::WriteAngles(int alpha1, int alpha2)
{
  int probe;
  if (alpha1 != angle1 || alpha2 != angle2)
  {
    if (alpha1 != angle1)
    {
      if (alpha1 > angle1)
        ++angle1;
      else
        --angle1;
      probe = angle1 + p1;
      //if (probe < 180 && probe > 0){
        servo1.write(probe);
      //}
      //else
        //return 1;
    }
    if (alpha2 != angle2)
    {
      if (alpha2 > angle2)
        ++angle2;
      else
        --angle2;
      probe = angle2 + p2;
      //if (probe < 180 && probe > 0){
        servo2.write(probe);
      //}
     // else
        //return 1;
    }
    return 0;
  }
  else
    return 1;
}

void TLeg::UpdateAngles(int &alpha1, int &alpha2)
{
  int R1 = round((float)qR / 10 * (R - L2)) + L2;
  long r2 = sqr(h) + sqr(R1);
  alpha1 = round(acos((float)(sqr(L2) + sqr(L1) - r2) / (2 * L1 * L2)) * ToGrad);
  alpha2 = 180 - round((acos((float)(h) / sqrt(r2)) + acos((float)(r2 + sqr(L1) - sqr(L2)) / (2 * sqrt(r2) * L1))) * ToGrad);
}

int TLeg::CalculateForStep(int d, int direction, int Radius)
{
  direction -= posOnBody;
  int newR = round(sqrt(Radius * Radius + d * d + 2 * d * Radius * cos(direction * ToRad)));
  R = Radius + round(0.6 * (newR - Radius));
  return 90 - round(ToGrad * asin(-d * sin(direction * ToRad) / newR));
}

inline int TLeg::ReadVoltage(){
  return analogRead(measurePin);
}
