#include <TimerThree.h>
#include "TSpider.h"
#include "Constants.h"

void ControlServices() {
  sei();
  Spider.CheckVcc();
  if (Spider.PositionAlignment() ||
      Spider.WorkloadsAlignment() ||
      Spider.HeightControl())
    Spider.BasicPosition();
  else
    Spider.UpdateAllAngles();
}

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

void TSpider::StartTimer(unsigned long miliseconds) {
  Timer3.initialize(miliseconds * 1000);
  Timer3.attachInterrupt(ControlServices);
  Timer3.start();
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

int TSpider::ChangeHeight(int delta, bool changeProperty = true)
{
  int error = 0;
  for (int i = 0; i < 6; ++i)
    error |= legs[i].ChangeHeight(delta);
  if (changeProperty) {
    height += delta;
    UpdateAllAngles();
  }
  return error;
}

void TSpider::BasicPosition()
{
  balanceActive = false;
  workloadsAlignemtActive = false;

  for (int i = 0; i < 6; i++)
    legs[i].ChangeHeight(-legs[i].GetHeight());
  height = 0;
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
  if ((newR >= minRadius) && (sqr(L1 + L2) > sqr(MaxHeight()) + sqr(newR)))
  {
    UpdateWorkloads();
    bool staying = false;
    for (int i = 0; i < 6; ++i)
      if (legs[i].workload > minWorkloadThreshold)
        staying = true;
    if (!staying)
    {
      for (int i = 0; i < 6; ++i)
        legs[i].R = newR;
      UpdateAllAngles();
      Radius = newR;
      return 0;
    }
    else {
      if (height < minLifting)
        return 3;
      Timer3.stop();
      for (int i = 0; i < 2; i++)
      {
        ThreeLegsUpDown( i, i + 2, i + 4, -1);
        legs[i].R = legs[i + 2].R = legs[i + 4].R = newR;
        UpdateAllAngles();

        ThreeLegsUpDown( i, i + 2, i + 4, 1);
        delay(stepDelaying);
        ReachGround();
        ControlServices();
      }
      Timer3.start();
      Radius = newR;
      return 0;
    }
  }
  else
    return 4;
}

int TSpider::Turn(int angle)
{
  if (angle <= maxTurn) {
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
      byte values[7] = {turnAngle, turnAngle, turnAngle, turnAngle, turnAngle, turnAngle, motionDelaying};
      for (int i = 0; i < 6; ++i)
        legs[i].R = newR;
      checkVccActive = false;
      board.TurnLegs(values);
      UpdateAllAngles();
      for (int i = 0; i < 6; ++i)
      {
        legs[i].R = Radius;
        values[i] = 90;
      }
      delay(1500);
      board.TurnLegs(values);
      UpdateAllAngles();
      checkVccActive = true;
      return 0;
    }
    return 4;
  }
  return 3;
}

int TSpider::FixedTurn(int angle)
{
  if (height < minLifting)
    return 3;
  int angle3, x, newR;
  int steps = ceil((float)abs(angle) / maxTurn);
  angle = sign(angle) * maxTurn;
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
    byte values[7] = {90, 90, 90, 90, 90, 90, motionDelaying};
    Timer3.stop();
    while (steps--) {
      for (int j = 1; j >= 0; --j)
      {
        int i = (sign ? 1 - j : j);
        values[i] = values[i + 2] = values[i + 4] = turnAngle;

        ThreeLegsUpDown( i, i + 2, i + 4, -1);

        board.TurnLegs(values);
        legs[i].R = legs[i + 2].R = legs[i + 4].R = newR;
        UpdateAllAngles();

        ThreeLegsUpDown( i, i + 2, i + 4, 1);
        delay(stepDelaying);
        ReachGround();
      }
      for (int i = 0; i < 6; ++i)
      {
        legs[i].R = Radius;
        values[i] = 90;
      }
      checkVccActive = false;
      board.TurnLegs(values);
      UpdateAllAngles();
      delay(stepDelaying);
      ControlServices();
      checkVccActive = true;
    }
    Timer3.start();
    return 0;
  }
  else
    return 4;
}

int TSpider::Move(int direction)
{
  if (height < minLifting)
    return 3;
  byte values[7] = {90, 90, 90, 90, 90, 90, motionDelaying};
  int i, a = 0;
  errno = 0;
  Timer3.stop();
  balanceActive = false;
  while (!esp.HasData() && !errno)
  {
    ThreeLegsUpDown(a, a + 2, a + 4, -1);
    for (i = a; i < 6; i += 2)
      values[i] = legs[i].CalculateForStep(stepLength, direction, Radius);
    for (i = (a + 1) % 2; i < 6; i += 2)
      values[i] = legs[i].CalculateForStep(stepLength, 180 + direction, Radius);

    board.TurnLegs(values);
    UpdateAllAngles();

    ThreeLegsUpDown(a, a + 2, a + 4, 1);
    delay(stepDelaying);
    ReachGround();
    ControlServices();
    a = (a + 1) % 2;
  }
  if (!errno) {
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
    delay(stepDelaying);
    ReachGround();
    ControlServices();

    ThreeLegsUpDown(1, 3, 5, -1);
    for (i = 1; i < 6; i += 2)
    {
      legs[i].R = Radius;
      values[i] = 90;
    }

    board.TurnLegs(values);
    UpdateAllAngles();

    ThreeLegsUpDown(1, 3, 5, 1);
    delay(stepDelaying);
    ReachGround();
    ControlServices();
    Timer3.start();
    balanceActive = true;
    return 0;
  }
  Timer3.start();
  return 1;
}

inline void TSpider::TwoLegsUpDown(int i, int j, int dir)
{
  legs[i].ChangeHeight(dir * height);
  legs[j].ChangeHeight(dir * height);
  UpdateAllAngles();
}

inline void TSpider::ThreeLegsUpDown(int i, int j, int k, int dir)
{
  legs[i].ChangeHeight(dir * height);
  TwoLegsUpDown(j, k, dir);
}

void TSpider::CheckVcc()
{
  if (checkVccActive)
    if (powerOn) {
      if (board.GetVcc() < 5500) {
        BasicPosition();
        PowerOff();
        powerOn = false;
        errno = 100;
      }
      else
        PowerOn();
    }
    else {
      BasicPosition();
      PowerOff();
    }
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
  return error;
}


int TSpider::PositionAlignment()
{
  if (balanceActive) {
    int error = 0;
    board.UpdatePosition();
    if ((abs(board.position.vertical - positionV) > maxSkew ||
         abs(board.position.horizontal - positionH) > 4 * maxSkew && positionV != 0))
      error = Balance();
    if (error)
      errno = 110 + error;
    return error;
  }
  return 0;
}

void TSpider::UpdateWorkloads() {
  unsigned int amount[6] = {0};
  for (int i = 0; i < 30; ++i)
    for (int j = 0; j < 6; ++j)
      amount[j] += legs[j].ReadVoltage();

  for (int i = 0; i < 6; ++i) {
    legs[i].workload = amount[i] / 30;
  }
}

int TSpider::ReachGround() {
  int error = 0;
  bool done;
  do {
    UpdateWorkloads();
    done = true;
    for (int i = 0; i < 6; ++i)
      if (legs[i].workload < minWorkloadThreshold) {
        done = false;
        error |= legs[i].ChangeHeight(4);
      }
    if (!done) {
      UpdateAllAngles();
      delay(60);
    }
  } while ( !error && !done);
  if (error)
    BasicPosition();
  return error;
}

int TSpider::GetUp(int h) {
  Timer3.stop();
  int error = ReachGround();
  if (!error) {
    height = MinHeight();
    error = ChangeHeight(h);
    if (!error) {
      balanceActive = true;
      workloadsAlignemtActive = true;
    }
  }
  Timer3.start();
  return error;
}

int TSpider::HeightControl() {
  if (heightControlActive) {
    int realHeight = MinHeight();
    if (height != realHeight) {
      int error = ChangeHeight(height - realHeight, false);
      if (error)
        errno = 130 + error;
      return error;
    }
  }
  return 0;
}

int TSpider::WorkloadsAlignment() {
  if (workloadsAlignemtActive) {
    UpdateWorkloads();
    unsigned long amount = 0;
    int error = 0;
    for (int i = 0; i < 6; ++i)
      amount += legs[i].workload;
    int avarageWorkload = amount / 6;
    for (int i = 0; i < 6; ++i) {
      error |= legs[i].ChangeHeight((int)((float)(avarageWorkload - legs[i].workload) / maxWorkloadDisparityRate / avarageWorkload));
    }
    if (error)
      errno = 120 + error;
    return error;
  }
  return 0;
}

String TSpider::HandleCurrentRequest() {
  switch (esp.currentRequest.requestType) {
    case esp.DO:
      return String(DoCommand());
      break;
    case esp.INFO:
      return GetInfo();
      break;
    case esp.SET:
      return String(SetProperty());
      break;
    case esp.ERR:
      return "Invalid request";
      break;
  }
}

int TSpider::DoCommand() {
  switch (esp.currentRequest.command_property)
  {
    case 'd':
      return SetRadius(esp.currentRequest.args[0]);
      break;
    case 'w':
      return ChangeHeight(esp.currentRequest.args[0]);
      break;
    case 'g':
      return GetUp(esp.currentRequest.args[0]);
      break;
    case 't':
      return FixedTurn(esp.currentRequest.args[0]);
      break;
    case 'y':
      return Turn(esp.currentRequest.args[0]);
      break;
    case 'z':
      BasicPosition();
      break;
    case 'm':
      return Move(esp.currentRequest.args[0]);
      break;
      case '|':{
          int error = legs[esp.currentRequest.args[0]].ChangeHeight(esp.currentRequest.args[1]);
          if (!error)
            UpdateAllAngles();
          return error;
      }
      break;
      case '-':{
          int newR = (legs[esp.currentRequest.args[0]].R + esp.currentRequest.args[1]);
          if ((newR >= minRadius) && (sqr(L1 + L2) > sqr(legs[esp.currentRequest.args[0]].GetHeight()) + sqr(newR))){
            legs[esp.currentRequest.args[0]].R = newR;
            UpdateAllAngles();
            return 0;
          }
          return 1;
      }
      break;
      case ')':{
          byte values[7] = {90, 90, 90, 90, 90, 90, motionDelaying};
          values[esp.currentRequest.args[0]] = esp.currentRequest.args[1];
          board.TurnLegs(values);
      }
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
      case 'e':
        result += String(errno);
        errno = 0;
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

int TSpider::SetProperty() {
  switch (esp.currentRequest.command_property)
  {
    case 'p':
      powerOn = !powerOn;
      break;
    case 'a':
      workloadsAlignemtActive = !workloadsAlignemtActive;
      break;
    case 'b':
      balanceActive = !balanceActive;
      break;
    case 'h':
      heightControlActive = !heightControlActive;
      break;
    default:
      return 9;
      break;
  }
  return 0;
}






























