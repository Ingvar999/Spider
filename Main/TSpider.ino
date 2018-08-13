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
  int newAngles[12] = {0};
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
}

int TSpider::ChangeHeight(int delta)
{
  int error = 0;
  for (int i = 0; i < 6; ++i)
    error |= legs[i].ChangeHeight(delta);
  UpdateAllAngles();
  return error;
}

void TSpider::BasicPosition()
{
  for (int i = 0; i < 6; i++)
    legs[i].ChangeHeight(-legs[i].GetHeight());
  UpdateAllAngles();
  byte values[7] = {90, 90, 90, 90, 90, 90, 15};
  board.TurnLegs(values);
  Radius = minRadius;
  for (int i = 0; i < 6; i++)
    legs[i].R = Radius;
  UpdateAllAngles();
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
    if (true)
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
  int dh = 0, error = 0;
  float tanV = tan(board.position.vertical * ToRad), tanPV = tan(positionV * ToRad);
  for (int i = 0; i < 6; ++i)
  {
    dh = round((Radius + a) * (cos((legs[i].GetPosition() - board.position.horizontal) * ToRad) * tanV -
                               cos((legs[i].GetPosition() - positionH) * ToRad) * tanPV));
    error |= legs[i].ChangeHeight(dh);
  }
  UpdateAllAngles();
  return error;
}


int TSpider::CheckBalance()
{
  int error = 0;
  if (balanceActive)
  {
    board.UpdatePosition();
    if ((abs(board.position.vertical - positionV) > maxSkew ||
         abs(board.position.horizontal - positionH) > 4 * maxSkew && positionV != 0))
      if (error = Balance())
        balanceActive = false;
  }
  return error;
}

int TSpider::Move(int direction)
{
  int error = 0;
    for (int i = 0; i < 6; ++i)
      if (legs[i].GetHeight() < lifting)
        return 3;
    byte values[7] = {90, 90, 90, 90, 90, 90, 10};
    int i, a = 0;
    //while (!esp.ReadRequest())
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
    //esp.Clear();
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

inline void TSpider::TwoLegsUpDown(int i, int j, int dir)
{
  legs[i].ChangeHeight(dir * lifting);
  legs[j].ChangeHeight(dir * lifting);
  UpdateAllAngles();
}

inline void TSpider::ThreeLegsUpDown(int i, int j, int k, int dir)
{
  legs[i].ChangeHeight(dir * lifting);
  TwoLegsUpDown(j, k, dir);
}

void TSpider::CheckVcc()
{
  if (board.GetVcc() < 5000) {
    BasicPosition();
    PowerOff();
  }
  else
    PowerOn();
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
      if (legs[i].workload < minWorkloadThreshold) {
        done = 0;
        error |= legs[i].ChangeHeight(3);
      }
    if (!done) {
      UpdateAllAngles();
      delay(40);
    }
  } while ( !error && !done);
  if (error)
    BasicPosition();
  return error;
}

int TSpider::WorkloadsAlignment() {
  if (workloadsAlignemtActive) {
    UpdateWorkloads();
    unsigned long amount = 0;
    int error = 0;
    for (int i = 0; i < 6; ++i)
      amount += legs[i].workload;
    int avarageWorkload = amount / 6;
    int maxWorkloadDisparity = maxWorkloadDisparityRate * avarageWorkload;
    for (int i = 0; i < 6; ++i) {
      if (legs[i].workload < minWorkloadThreshold)
        error |= legs[i].ChangeHeight(3);
      else if (abs(legs[i].workload - avarageWorkload) > maxWorkloadDisparity)
        error |= legs[i].ChangeHeight(2 * sign(avarageWorkload - legs[i].workload));
    }
    UpdateAllAngles();
  }
}

String TSpider::HandleCurrentRequest() {
  switch (esp.currentRequest.requestType) {
    case esp.DO:
      return String(DoCommand());
      break;
    case esp.INFO:
      return GetInfo();
      break;
    case esp.ERR:
      return "Invalid request";
      break;
  }
}

int TSpider::DoCommand() {
  switch (esp.currentRequest.command)
  {
    case 'd':
      return Spider.SetRadius(esp.currentRequest.args[0]);
      break;
    case 'w':
      return Spider.ChangeHeight(esp.currentRequest.args[0]);
      break;
    case 'g':
      return Spider.ReachGround();
      break;
    case 't':
      return Spider.FixedTurn(esp.currentRequest.args[0]);
      break;
    case 'y':
      return Spider.Turn(esp.currentRequest.args[0]);
      break;
    case 'b':
      Spider.balanceActive = !Spider.balanceActive;
      break;
    case 'a':
      Spider.workloadsAlignemtActive = !Spider.workloadsAlignemtActive;
      break;
    case 'z':
      Spider.BasicPosition();
      break;
    case 'm':
      return Spider.Move(esp.currentRequest.args[0]);
      break;
    default:
      return 9;
      break;
  }
  return 0;
}

String TSpider::GetInfo() {
  String result;
  for (int i = 0; i < esp.currentRequest.requiredValues.length(); ++i) {
    switch (esp.currentRequest.requiredValues[i]) {
      case 'v':
        result += String(board.GetVcc());
        break;
      case 'r':
        UpdateWorkloads();
        for (int i = 0; i < 6; ++i)
          result += String(legs[i].workload) + ' ';
        break;
      case 'p':
        board.UpdatePosition();
        result += String(board.position.vertical) + ' ' + String(board.position.horizontal);
        break;
      default:
        result += "Undefined";
        break;
    }
    if (i + 1 < esp.currentRequest.requiredValues.length())
      result += "\r\n";
  }
  return result;
}






























