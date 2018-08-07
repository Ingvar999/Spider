#include "TSpider.h"
#include "Constants.h"

inline void TSpider::PowerOn() {
  digitalWrite(powerPin, HIGH);
}

inline void TSpider::PowerOff() {
  digitalWrite(powerPin, LOW);
}

void TSpider::Init() {
  pinMode(powerPin, OUTPUT);
  PowerOff();

  analogReference(INTERNAL1V1);
}

void TSpider::InitLeg(int i, int _pos, int pinCont, int pin1, int pin2, int pog1 = 0, int pog2 = 0, int _qR = 10)
{
  legs[i].Init(_pos, pinCont, pin1, pin2, pog1, pog2, _qR);
  legs[i].R = Radius;
}

void TSpider::UpdateAllAngles()
{
  int *newAngles = new int[12] {0};
  int j = 0, i = 0;
  for (i = 0; i < 6; i++)
  {
    legs[i].UpdateAngles(newAngles[j], newAngles[j + 1]);
    j += 2;
  }
  int done = 1;
  do
  {
    j = 0;
    done = 1;
    for (i = 0; i < 6; i++)
    {
      done &= legs[i].WriteAngles(newAngles[j], newAngles[j + 1]);
      j += 2;
    }
    delay(motionDelaying);
  } while (!done);
  delete[] newAngles;
  lastBalancingTime = millis();
}

int TSpider::ChangeHeight(int dH)
{
  int error = 0;
  for (int i = 0; i < 6; ++i)
    error |= legs[i].SetHeight(legs[i].GetHeight() + dH);
  UpdateAllAngles();
  return error;
}

void TSpider::BasicPosition()
{
  for (int i = 0; i < 6; i++)
    legs[i].SetHeight(0);
  UpdateAllAngles();
  byte values[7] = {90, 90, 90, 90, 90, 90, 15};
  board.TurnLegs(values);
  for (int i = 0; i < 6; i++)
    legs[i].R = 30;
  UpdateAllAngles();
  Radius = 30;
}

int TSpider::MaxHeight()
{
  int res = legs[0].GetHeight();
  for (int i = 1; i < 6; ++i)
    if (legs[i].GetHeight() > res)
      res = legs[i].GetHeight();
  return res;
}

int TSpider::MinHeight()
{
  int res = legs[0].GetHeight();
  for (int i = 1; i < 6; ++i)
    if (legs[i].GetHeight() < res)
      res = legs[i].GetHeight();
  return res;
}

int TSpider::SetRadius(int newR)
{
  int error = 0;
  if ((newR >= minRadius) && (sqr(L1 + L2) > sqr(MaxHeight()) + sqr(newR)))
  {
    //UpdateContacts();
    if (contacts == 0)
    {
      for (int i = 0; i < 6; ++i)
        legs[i].R = newR;
      UpdateAllAngles();
      Radius = newR;
      return 0;
    }
    else if ((error /*= toContacts()*/) == 0)
    {
      for (int i = 0; i < 6; ++i)
        if (legs[i].GetHeight() < lifting)
          return 4;
      for (int i = 0; i < 2; i++)
      {
        ThreeLegsUpDown( i, i + 2, i + 4, -1);
        legs[i].R = legs[i + 2].R = legs[i + 4].R = newR;
        UpdateAllAngles();

        ThreeLegsUpDown( i, i + 2, i + 4, 1);

        //error = toContacts();
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
  int error; // = toContacts();
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
      byte values[7] = {turnAngle, turnAngle, turnAngle, turnAngle, turnAngle, turnAngle, 7};
      for (int i = 0; i < 6; ++i)
        legs[i].R = newR;
      board.TurnLegs(values);
      UpdateAllAngles();
      for (int i = 0; i < 6; ++i)
      {
        legs[i].R = Radius;
        values[i] = 90;
      }
      delay(2000);
      board.TurnLegs(values);
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
  int error;// = toContacts();
  if (error == 0)
  {
    for (int i = 0; i < 6; ++i)
      if (legs[i].GetHeight() < lifting)
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
      byte values[7] = {90, 90, 90, 90, 90, 90, 7};
      for (int j = 1; j >= 0; --j)
      {
        int i = (sign ? 1 - j : j);
        values[i] = values[i + 2] = values[i + 4] = turnAngle;
        ThreeLegsUpDown( i, i + 2, i + 4, -1);
        board.TurnLegs(values);
        legs[i].R = legs[i + 2].R = legs[i + 4].R = newR;
        UpdateAllAngles();
        ThreeLegsUpDown( i, i + 2, i + 4, 1);
//        toContacts();
        delay(100);
      }
      for (int i = 0; i < 6; ++i)
      {
        legs[i].R = Radius;
        values[i] = 90;
      }
      board.TurnLegs(values);
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
  int error;// = toContacts();
  if (error == 0)
  {
    int dh = 0;
    float tanV = tan(board.position.vertical * ToRad), tanPV = tan(positionV * ToRad);
    for (int i = 0; i < 6; ++i)
    {
      dh = round((Radius + a) * (cos((legs[i].GetPosition() - board.position.horizontal) * ToRad) * tanV -
                                 cos((legs[i].GetPosition() - positionH) * ToRad) * tanPV));
      error |= legs[i].SetHeight(legs[i].GetHeight() + dh);
    }
    UpdateAllAngles();
    return error;
  }
  else
    return error;
}


int TSpider::CheckBalance()
{
  int error = 0;
  if (millis() - lastBalancingTime > balancingInterval)
  {
    board.UpdatePosition();
    if (balancing && ((abs(board.position.vertical - positionV) > maxSkew ||
                       abs(board.position.horizontal - positionH) > 4 * maxSkew && positionV != 0)))
//      if (!GetContacts())
  //      balancing = false;
      /*else*/ if (error = Balance())
        balancing = false;
    lastBalancingTime = millis();
  }
  return error;
}

int TSpider::Move(int direction)
{
  int error;// = toContacts();
  if (error == 0)
  {
    for (int i = 0; i < 6; ++i)
      if (legs[i].GetHeight() < lifting)
        return 3;
    byte values[7] = {90, 90, 90, 90, 90, 90, 10};
    int i, a = 0;
    while (!esp.hasData())
    {
      ThreeLegsUpDown(a, a + 2, a + 4, -1);

      for (i = a; i < 6; i += 2)
        values[i] = legs[i].CalculateForStep(stepLength, direction, Radius);
      for (i = (a + 1) % 2; i < 6; i += 2)
        values[i] = legs[i].CalculateForStep(stepLength, 180 + direction, Radius);

      board.TurnLegs(values);
      UpdateAllAngles();

      ThreeLegsUpDown(a, a + 2, a + 4, 1);
      //toContacts();
      delay(50);
      a = (a + 1) % 2;
    }
    esp.Clear();
    ThreeLegsUpDown(0, 2, 4, -1);
    for (i = 0; i < 6; i += 2)
    {
      legs[i].R = Radius;
      values[i] = 90;
    }
    
    board.TurnLegs(values);
    UpdateAllAngles();
    
    ThreeLegsUpDown(0, 2, 4, 1);
    delay(50);
    ThreeLegsUpDown(1, 3, 5, -1);
    for (i = 1; i < 6; i += 2)
    {
      legs[i].R = Radius;
      values[i] = 90;
    }
    
    board.TurnLegs(values);
    UpdateAllAngles();
    
    ThreeLegsUpDown(1, 3, 5, 1);
    return 0;
  }
  else
    return error;
}

inline void TSpider::TwoLegsUpDown(int i, int j, int dir)
{
  legs[i].SetHeight(legs[i].GetHeight() + dir * lifting);
  legs[j].SetHeight(legs[j].GetHeight() + dir * lifting);
  UpdateAllAngles();
}

inline void TSpider::ThreeLegsUpDown(int i, int j, int k, int dir)
{
  legs[i].SetHeight(legs[i].GetHeight() + dir * lifting);
  TwoLegsUpDown(j, k, dir);
}

void TSpider::CheckVcc()
{
  if (board.GetVcc() < 5000) {
    BasicPosition();
    PowerOff();
  }
}

void TSpider::UpdateWorkloads() {
  unsigned int amount[6] = {0};
  for (int i = 0; i < 30; ++i)
    for (int j = 0; j < 6; ++j)
      amount[j] += legs[j].ReadVoltage();

  for (int i = 0; i < 6; ++i) {
    legs[i].workload = amount[i] / 30;
    Serial.print(legs[i].workload);
    Serial.print(' ');
  }
  Serial.println();
}

int TSpider::ReachGround() {
  int error = 0, done;
  do {
    UpdateWorkloads();
    done = 1;
    for (int i = 0; i < 6; ++i)
      if (legs[i].workload < 50) {
        done = 0;
        error |= legs[i].SetHeight(legs[i].GetHeight() + 2);
      }
    if (!done)
      UpdateAllAngles();
      //delay(20);
  } while ( !error && !done);
  if (error)
    BasicPosition();
  return error;
}
































