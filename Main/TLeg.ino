#include "TLeg.h"
#include "Constants.h"

void TLeg::Init(int _pos, int pinCont, int pin1, int pin2, int pog1, int pog2, int _qR)
{
  pos = _pos;
  qR = _qR;
  p1 = pog1;
  p2 = pog2;
  servo1.attach(pin1);
  servo2.attach(pin2);
  angle1 = servo1.read();
  angle2 = servo2.read();
  contactPin = pinCont;
  pinMode(contactPin, INPUT);
}

int TLeg::SetHeight(int newh)
{
  if ((newh >= 0) && (sqr(L1 + L2) > sqr(newh) + sqr(R)))
  {
    h = newh;
    return 0;
  }
  else
    return 1;
}

int TLeg::WriteAngle(int alpha1, int alpha2)
{
  if (alpha1 != angle1 || alpha2 != angle2)
  {
    if (alpha1 != angle1)
    {
      if (alpha1 > angle1)
        ++angle1;
      else
        --angle1;
      servo1.write(angle1 + p1);
    }
    if (alpha2 != angle2)
    {
      if (alpha2 > angle2)
        ++angle2;
      else
        --angle2;
      servo2.write(angle2 + p2);
    }
    return 0;
  }
  else
    return 1;
}

void TLeg::UpdateAngle(int &alpha1, int &alpha2)
{
  int R1 = round((float)qR / 10 * (R - L2)) + L2;
  long r2 = sqr(h) + sqr(R1);
  alpha1 = round(acos((float)(sqr(L2) + sqr(L1) - r2) / (2 * L1 * L2)) * ToGrad);
  alpha2 = 180 - round((acos((float)(h) / sqrt(r2)) + acos((float)(r2 + sqr(L1) - sqr(L2)) / (2 * sqrt(r2) * L1))) * ToGrad);
}

int TLeg::ForStep(int d, int a, int Radius)
{
  a -= pos;
  int newR = round(sqrt(Radius * Radius + d * d + 2 * d * Radius * cos(a * ToRad)));
  R = Radius + round(0.6 * (newR - Radius));
  return 90 - round(ToGrad * asin(-d * sin(a * ToRad) / newR));
}


