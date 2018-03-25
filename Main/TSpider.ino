#include "Constants.h"
#include "TSpider.h"
#include "Gyro.h"

void TSpider::Init(int i, int _pos, int pinCont, int pin1, int pin2, int pog1 = 0, int pog2 = 0, int _qR = 10)
{
  legs[i].Init(_pos, pinCont, pin1, pin2, pog1, pog2, _qR);
  legs[i].R = Radius;
}

extern unsigned long int oldTimeB;

void TSpider::UpdateAllAngles()
{
  int *newAngles = new int[12] {0};
  int j = 0, i = 0;
  for (i = 0; i < 6; i++)
  {
    legs[i].UpdateAngle(newAngles[j], newAngles[j + 1]);
    j += 2;
  }
  int done = 1;
  do
  {
    j = 0;
    done = 1;
    for (i = 0; i < 6; i++)
    {
      done &= legs[i].WriteAngle(newAngles[j], newAngles[j + 1]);
      j += 2;
    }
    CheckGyro();
    delay(10);
  } while (!done);
  delete[] newAngles;
  oldTimeB = millis();
}

int TSpider::ChangeHeight(int dH)
{
  int error = 0;
  for (int i = 0; i < 6; ++i)
    error |= legs[i].SetHeight(legs[i].GetHeight() + dH);
  UpdateAllAngles();
  return error;
}

int TSpider::GetContacts()
{
  UpdateContacts();
  return contacts;
}

byte TSpider::ReadContacts()
{
  byte temp;
  for (int i = 0; i < 6; ++i)
    if (legs[i].HasContact())
      temp |= bit(i);
  return temp;
}

void TSpider::BasicPosition()
{
  for (int i = 0; i < 6; i++)
    legs[i].SetHeight(0);
  UpdateAllAngles();
  byte value[7] = {90, 90, 90, 90, 90, 90, 15};
  Serial2.write('w');
  Serial2.write(value, 7);
  for (int i = 0; i < 6; i++)
    legs[i].R = 30;
  UpdateAllAngles();
  Radius = 40;
}

void TSpider::UpdateContacts()
{
  contacts = 0;
  for (int i = 0; i < 6; ++i)
    if (legs[i].HasContact())
      ++contacts;
}

int TSpider::MaxHeight()
{
  int res = legs[0].GetHeight();
  for (int i = 1; i < 6; ++i)
    if (legs[i].GetHeight() > res)
      res = legs[i].GetHeight();
  return res;
}

int TSpider::toContacts()
{
  UpdateContacts();
  int oldC = contacts;
  while (contacts < 6)
  {
    for (int i = 0; i < 6; ++i)
    {
      if (!legs[i].HasContact())
        if (legs[i].SetHeight(legs[i].GetHeight() + 1) != 0)
          return 1;
    }
    UpdateAllAngles();
    UpdateContacts();
    if (contacts < oldC)
      return 2;
    else
      oldC = contacts;
  }
  return 0;
}

int TSpider::SetRadius(int newR)
{
  int error = 0;
  if ((newR >= minRadius) && (sqr(L1 + L2) > sqr(MaxHeight()) + sqr(newR)))
  {
    UpdateContacts();
    if (contacts == 0)
    {
      for (int i = 0; i < 6; ++i)
        legs[i].R = newR;
      UpdateAllAngles();
      Radius = newR;
      return 0;
    }
    else if ((error = toContacts()) == 0)
    {
      for (int i = 0; i < 6; ++i)
        if (legs[i].GetHeight() < Lifting)
          return 4;
      for (int i = 0; i < 2; i++)
      {
        ThreeLegsUpDown( i, i + 2, i + 4, -1);
        legs[i].R = legs[i + 2].R = legs[i + 4].R = newR;
        UpdateAllAngles();

        ThreeLegsUpDown( i, i + 2, i + 4, 1);

        error = toContacts();
      }
      Radius = newR;
      return error;
    }
    else
      return error;
  }
  else
    return 3;
}

int TSpider::Turn(int angle)
{
  int error = toContacts();
  if (error == 0)
  {
    int angle3, x, newR;
    angle3 = 90 - (angle >> 1);
    x = round(3 * a * angle / 90.0);
    newR = round(sqrt(sqr(GetRadius()) + sqr(x) - 2 * x * GetRadius() * cos(angle3 * ToRad)));
    if (sqr(L1 + L2) > sqr(MaxHeight()) + sqr(newR))
    {
      bool sign = angle < 0;
      byte turnAngle = round(ToGrad * asin(GetRadius() * sin(angle3 * ToRad) / newR));
      if (sign)
        turnAngle = 180 - turnAngle;
      newR = Radius + round(0.6 * (newR - Radius));
      //newR += round(al * cos(turnAngle*ToRad));
      byte newVal[7] = {turnAngle, turnAngle, turnAngle, turnAngle, turnAngle, turnAngle, 7};
      for (int i = 0; i < 6; ++i)
        legs[i].R = newR;
      Serial2.write('w');
      Serial2.write(newVal, 7);
      UpdateAllAngles();
      for (int i = 0; i < 6; ++i)
      {
        legs[i].R = Radius;
        newVal[i] = 90;
      }
      Delay(2000);
      Serial2.write('w');
      Serial2.write(newVal, 7);
      UpdateAllAngles();

      return 0;
    }
    else
      return 4;
  }
  else
    return error;
}

int TSpider::FixedTurn(int angle)
{
  int error = toContacts();
  if (error == 0)
  {
    for (int i = 0; i < 6; ++i)
      if (legs[i].GetHeight() < Lifting)
        return 3;
    int angle3, x, newR;
    angle3 = 90 - (angle >> 1);
    x = round(3 * a * angle / 90.0);
    newR = round(sqrt(sqr(GetRadius()) + sqr(x) - 2 * x * GetRadius() * cos(angle3 * ToRad)));
    if (sqr(L1 + L2) > sqr(MaxHeight()) + sqr(newR))
    {
      bool sign = angle < 0;
      byte turnAngle = round(ToGrad * asin(GetRadius() * sin(angle3 * ToRad) / newR));
      newR = Radius + round(0.6 * (newR - Radius));
      if (sign)
        turnAngle = 180 - turnAngle;
      byte newVal[7] = {90, 90, 90, 90, 90, 90, 7};
      for (int j = 1; j >= 0; --j)
      {
        int i = (sign ? 1 - j : j);
        newVal[i] = newVal[i + 2] = newVal[i + 4] = turnAngle;
        ThreeLegsUpDown( i, i + 2, i + 4, -1);
        Serial2.write('w');
        Serial2.write(newVal, 7);
        legs[i].R = legs[i + 2].R = legs[i + 4].R = newR;
        UpdateAllAngles();
        ThreeLegsUpDown( i, i + 2, i + 4, 1);
        toContacts();
        Delay(100);
      }
      for (int i = 0; i < 6; ++i)
      {
        legs[i].R = Radius;
        newVal[i] = 90;
      }
      Serial2.write('w');
      Serial2.write(newVal, 7);
      UpdateAllAngles();

      return 0;
    }
    else
      return 4;
  }
  else
    return error;
}

int TSpider::Balance()
{
  int error = toContacts();
  if (error == 0)
  {
    SerialX.print(horizontal);
    SerialX.print(" / ");
    SerialX.println(vertical);
    int dh = 0;
    float tanV = tan(vertical * ToRad), tanPV = tan(positionV * ToRad);
    for (int i = 0; i < 6; ++i)
    {
      dh = round((Radius + a) * (cos((legs[i].GetPosition() - horizontal) * ToRad) * tanV -
                                 cos((legs[i].GetPosition() - positionH) * ToRad) * tanPV));
      error |= legs[i].SetHeight(legs[i].GetHeight() + dh);
    }
    UpdateAllAngles();
    return error;
  }
  else
    return error;
}

unsigned long int oldTimeB = millis();
int TSpider::CheckBalance()
{
  int error = 0;
  if (millis() - oldTimeB > IntervalB)
  {
    if (balancing && ((abs(vertical - positionV) > MaxSkew || abs(horizontal - positionH) > 4 * MaxSkew && positionV != 0)))
      if (!GetContacts())
        balancing = false;
      else if (error = Balance())
        balancing = false;
    oldTimeB = millis();
  }
  return error;
}

int TSpider::Move(int direction)
{
  int error = toContacts();
  if (error == 0)
  {
    for (int i = 0; i < 6; ++i)
      if (legs[i].GetHeight() < Lifting)
        return 3;
    byte Val[7] = {90, 90, 90, 90, 90, 90, 10};
    int i, a = 0;
    SerialX.readString();
    while (SerialX.available() <= 0)
    {
      ThreeLegsUpDown(a, a + 2, a + 4, -1);

      for (i = a; i < 6; i += 2)
        Val[i] = legs[i].ForStep(dS, direction, Radius);
      for (i = (a + 1) % 2; i < 6; i += 2)
        Val[i] = legs[i].ForStep(dS, 180 + direction, Radius);

      Serial2.write('w');
      Serial2.write(Val, 7);
      UpdateAllAngles();

      ThreeLegsUpDown(a, a + 2, a + 4, 1);
      toContacts();
      Delay(50);
      a = (a + 1) % 2;
    }
    SerialX.read();
    ThreeLegsUpDown(0, 2, 4, -1);
    for (i = 0; i < 6; i += 2)
    {
      legs[i].R = Radius;
      Val[i] = 90;
    }
    Serial2.write('w');
    Serial2.write(Val, 7);
    UpdateAllAngles();
    ThreeLegsUpDown(0, 2, 4, 1);
    Delay(50);
    ThreeLegsUpDown(1, 3, 5, -1);
    for (i = 1; i < 6; i += 2)
    {
      legs[i].R = Radius;
      Val[i] = 90;
    }
    Serial2.write('w');
    Serial2.write(Val, 7);
    UpdateAllAngles();
    ThreeLegsUpDown(1, 3, 5, 1);
    return 0;
  }
  else
    return error;
}
inline void TSpider::TwoLegsUpDown(int i, int j, int a)
{
  legs[i].SetHeight(legs[i].GetHeight() + a * Lifting);
  legs[j].SetHeight(legs[j].GetHeight() + a * Lifting);
  UpdateAllAngles();
}

inline void TSpider::ThreeLegsUpDown(int i, int j, int k, int a)
{
  legs[i].SetHeight(legs[i].GetHeight() + a * Lifting);
  legs[j].SetHeight(legs[j].GetHeight() + a * Lifting);
  legs[k].SetHeight(legs[k].GetHeight() + a * Lifting);
  UpdateAllAngles();
}
































