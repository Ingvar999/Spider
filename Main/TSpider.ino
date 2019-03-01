#include <TimerThree.h>
#include "TSpider.h"
#include "Constants.h"

void TSpider::Wander() {
  SetRadius(50);
  if (errno == OK) {
    ChangeHeight(40);
    while (errno == OK) {
      Move(majorDirection);
      while ( errno == 0 && !isValidDistance()) {
        FixedTurn(60);
      }
    }
  }
  BasicPosition();
}

void TSpider::ControlServices() {
  CheckLight();
  CheckVcc();
  Update_OnSurface_Worklods_Position();
  if (onSurface && errno == OK) {
    if (!(PositionAlignment() || WorkloadsAlignment() || HeightControl()))
      UpdateAllAngles();
  }
  if (esp.ReadRequest()) {
    esp.SendResponse(HandleCurrentRequest());
  }
}

void TimerHandler() {
  sei();
  Spider.ControlServices();
}

inline void TSpider::PowerOn() {
  digitalWrite(powerPin, HIGH);
  powerOn = true;
}

inline void TSpider::PowerOff() {
  digitalWrite(powerPin, LOW);
  powerOn = false;
}

void TSpider::Init() {
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);

  pinMode(lightDetectionPin, INPUT);
  pinMode(ledPin, OUTPUT);
  analogReference(INTERNAL1V1);
  sonar = new NewPing(trigPin, echoPin, 150);
}

void TSpider::StartTimer(unsigned long miliseconds) {
  Timer3.initialize(miliseconds * 1000);
  Timer3.attachInterrupt(TimerHandler);
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
  //производит рассчёт новых углов для каждой ноги
  for (i = 0; i < 6; i++)
  {
    legs[i].UpdateAngles(newAngles[j], newAngles[j + 1]);
    j += 2;
  }
  int done = 1;
  //физически изменяет значения
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

//второй параметр используется при контроле высоты
void TSpider::ChangeHeight(int delta, bool changeProperty = true)
{
  int error = 0;
  if (changeProperty && !onSurface) {
    ReachGround();
    if (errno == OK) {
      height = MinHeight();
    }
  }
  if (errno == OK) {
    for (int i = 0; i < 6; ++i)
      error |= legs[i].ChangeHeight(delta);
    if (error) {
      SetErrno(LEG_CANNOT_REACH_POINT);
    }
    else if (changeProperty) {
      height += delta;
      UpdateAllAngles();
    }
  }
}

void TSpider::BasicPosition()
{
  Timer3.stop();
  for (int i = 0; i < 6; i++)
    legs[i].ChangeHeight(-legs[i].GetHeight());
  height = 0;
  UpdateAllAngles();

  byte values[7] = {90, 90, 90, 90, 90, 90, 10};
  board.TurnLegs(values);

  Radius = minRadius;
  for (int i = 0; i < 6; i++)
    legs[i].R = Radius;
  UpdateAllAngles();
  Timer3.start();
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

void TSpider::SetRadius(int newR)
{
  if ((newR >= minRadius) && (sqr(L1 + L2) > sqr(MaxHeight()) + sqr(newR)))
  {
    if (!onSurface)
    {
      for (int i = 0; i < 6; ++i)
        legs[i].R = newR;
      UpdateAllAngles();
      Radius = newR;
    }
    else {
      if (height >= minLifting) {
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
        Radius = newR;
        Timer3.start();
      }
      else
        SetErrno(TOO_SMALL_HEIGHT);
    }
  }
  else
    SetErrno(LEG_CANNOT_REACH_POINT);
}

void TSpider::Turn(int angle)
{
  angle = (angle <= maxTurn ? angle : maxTurn);
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
    //checkVccActive = false;
    board.TurnLegs(values);
    UpdateAllAngles();
    for (int i = 0; i < 6; ++i)
    {
      legs[i].R = Radius;
      values[i] = 90;
    }
    delay(stepDelaying);
    board.TurnLegs(values);
    UpdateAllAngles();
    //checkVccActive = true;
  }
  else
    SetErrno(LEG_CANNOT_REACH_POINT);
}

void TSpider::FixedTurn(int angle)
{
  if (!(height < minLifting)) {
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
        //checkVccActive = false;
        board.TurnLegs(values);
        UpdateAllAngles();
        delay(stepDelaying);
        ControlServices();
        //checkVccActive = true;
      }
      Timer3.start();
    }
    else
      SetErrno(LEG_CANNOT_REACH_POINT);
  }
  else
    SetErrno(TOO_SMALL_HEIGHT);
}

void TSpider::Move(int direction)
{
  if (height >= minLifting) {
    byte values[7] = {90, 90, 90, 90, 90, 90, motionDelaying};
    int i, a = 0;
    Timer3.stop();
    while (tasksQueue.isEmpty() && errno == OK)
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
    if (errno == OK) {
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
    }
    Timer3.start();
  }
  else
    SetErrno(TOO_SMALL_HEIGHT);
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
      if (board.GetVcc() < minVoltage) {
        SetErrno(LOW_BATTARY);
        PowerOff();
      }
      else
        PowerOn();
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
  if (error) {
    SetErrno(LEG_CANNOT_REACH_POINT);
  }
  return error;
}


int TSpider::PositionAlignment()
{
  int error = 0;
  if (balanceActive) {
    if ((abs(board.position.vertical - positionV) > maxSkew ||
         abs(board.position.horizontal - positionH) > 4 * maxSkew && positionV != 0))
      error = Balance();
    return error;
  }
  return error;
}

int TSpider::UpdateWorkloads() {
  unsigned long amount[6] = {0};
  for (int i = 0; i < 30; ++i)
    for (int j = 0; j < 6; ++j)
      amount[j] += legs[j].ReadVoltage();

  for (int i = 0; i < 6; ++i) {
    legs[i].workload = amount[i] / 30;
    if (legs[i].workload > maxWorkloadThreshold) {
      SetErrno(HIGH_WORKLOAD);
      return 1;
    }
  }
  return 0;
}

void TSpider::ReachGround() {
  int error = 0;
  bool done;
  do {
    UpdateWorkloads();
    done = true;
    for (int i = 0; i < 6; ++i) {
      if (legs[i].workload < minWorkloadThreshold) {
        done = false;
        error |= legs[i].ChangeHeight(4);
      }
    }
    if (!done) {
      UpdateAllAngles();
      delay(60);
    }
  } while ( !error && !done);
  if (error)
    SetErrno(LEG_CANNOT_REACH_POINT);
  return error;
}

int TSpider::HeightControl() {
  if (heightControlActive) {
    int realHeight = MinHeight();
    if (height != realHeight) {
      ChangeHeight(height - realHeight, false);
      return errno;
    }
  }
  return 0;
}

int TSpider::WorkloadsAlignment() {
  if (workloadsAlignemtActive) {
    unsigned int amount = 0;
    int error = 0;
    for (int i = 0; i < 6; ++i)
      amount += legs[i].workload;
    int avarageWorkload = amount / 6;
    for (int i = 0; i < 6; ++i) {
      error |= legs[i].ChangeHeight((int)((float)(avarageWorkload - legs[i].workload) / (maxWorkloadDisparityRate * avarageWorkload)));
    }
    if (error)
      SetErrno(LEG_CANNOT_REACH_POINT);
    return error;
  }
  return 0;
}

String TSpider::HandleCurrentRequest() {
  switch (esp.currentRequest.requestType) {
    case esp.DO:
      if (errno != POWER_OFF && errno != LOW_BATTARY) {
        if (!tasksQueue.isFull()) {
          TTask task;
          task.command = esp.currentRequest.command_property;
          task.args[0] = esp.currentRequest.args[0];
          task.args[1] = esp.currentRequest.args[1];
          task.argc = esp.currentRequest.argc;
          tasksQueue.Push(task);
          errno = OK;
          return "Recieved";
        }
        else
          return "Queue is full";
      }
      else {
        return GetErrorMessage();
      }
      break;
    case esp.INFO:
      return GetInfo();
      break;
    case esp.SET:
      return SetProperty();
      break;
    case esp.ERR:
      return "Invalid request type";
      break;
  }
}

void TSpider::DoCommands() {
  if (!tasksQueue.isEmpty()) {
    const TTask& task = tasksQueue.Pop();
    switch (task.argc) {
      case 0:
        switch (task.command)
        {
          case 'b':
            BasicPosition();
            break;
          case 'm':
            Move(majorDirection);
            break;
        }
        break;
      case 1:
        switch (task.command)
        {
          case 'r':
            SetRadius(task.args[0]);
            break;
          case 'h':
            ChangeHeight(task.args[0]);
            break;
          case 'f':
            FixedTurn(task.args[0]);
            break;
          case 't':
            Turn(task.args[0]);
            break;
          case 'm':
            Move(task.args[0]);
            break;
        }
        break;
      case 2:
        switch (task.command)
        {
          case '|': {
              int error = legs[task.args[0]].ChangeHeight(task.args[1]);
              if (!error)
                UpdateAllAngles();
            }
            break;
          case '-': {
              int newR = (legs[task.args[0]].R + task.args[1]);
              if ((newR >= minRadius) && (sqr(L1 + L2) > sqr(legs[task.args[0]].GetHeight()) + sqr(newR))) {
                legs[task.args[0]].R = newR;
                UpdateAllAngles();
              }
            }
            break;
          case ')': {
              byte values[7] = {90, 90, 90, 90, 90, 90, motionDelaying};
              values[task.args[0]] = task.args[1];
              board.TurnLegs(values);
            }
            break;
        }
        break;
    }
  }
}

String TSpider::GetInfo() {
  String result;
  for (int i = 0; i < esp.currentRequest.requiredValues.length(); ++i) {
    switch (esp.currentRequest.requiredValues[i]) {
      case 'h':
        result += String(height);
        break;
      case 'r':
        result += String(Radius);
        break;
      case 'v':
        result += String(board.GetVcc());
        break;
      case 'w':
        for (int i = 0; i < 5; ++i)
          result += String(legs[i].workload) + ' ';
        result += String(legs[5].workload);
        break;
      case 'p':
        result += String(board.position.vertical) + ' ' + String(board.position.horizontal);
        break;
      case 'e':
        result += GetErrorMessage();
        break;
      case 'd':
        result += String(sonar->ping_cm());
        break;
      case 'i':
        result += BoolToString(powerOn);
        break;
      case 'c':
        result += BoolToString(balanceActive) + BoolToString(workloadsAlignemtActive) +
                  BoolToString(heightControlActive) + BoolToString(checkVccActive) + BoolToString(lightControlActive);
        break;
      default:
        result += "Undefined";
        break;
    }
    if (i + 1 < esp.currentRequest.requiredValues.length())
      result += "\n";
  }
  return result;
}

String TSpider::SetProperty() {
  switch (esp.currentRequest.command_property)
  {
    case 'i':
      bool toOn = (bool)esp.currentRequest.args[0];
      if (toOn) {
        PowerOn();
        if (errno == POWER_OFF) {
          errno = OK;
        }
      }
      else {
        SetErrno(POWER_OFF);
        PowerOff();
      }
      break;
    case 'w':
      workloadsAlignemtActive = esp.currentRequest.args[0];
      break;
    case 'b':
      balanceActive = esp.currentRequest.args[0];
      break;
    case 'h':
      heightControlActive = esp.currentRequest.args[0];
      break;
    case 'c':
      checkVccActive = esp.currentRequest.args[0];
      if (!checkVccActive && errno == LOW_BATTARY) {
        errno = OK;
      }
      break;
    case 'l':
      lightControlActive = esp.currentRequest.args[0];
      break;
    case 's':
      motionDelaying = 50 - esp.currentRequest.args[0];
      break;
    case 'p':
      positionV = esp.currentRequest.args[0];
      positionH = esp.currentRequest.args[1];
      break;
    default:
      return "Invalid property";
      break;
  }
  return "Recieved";
}

void TSpider::CheckLight() {
  static int stableCounter = 0;
  static const int maxCount = 3;
  if (lightControlActive) {
    if (!digitalRead(lightDetectionPin)) {
      if (isLightning) {
        if (++stableCounter == maxCount) {
          TurnLight(LOW);
          stableCounter = 0;
        }
      }
    }
    else {
      if (!isLightning) {
        TurnLight(HIGH);
      }
    }
  }
}

void TSpider::TurnLight(int state) {
  isLightning = (bool)state;
  digitalWrite(ledPin, state);
}

String TSpider::GetErrorMessage() {
  static const String ErrorMessages[] = {"OK", "Problem with a subboard", "Low battary for moving", "Legs power is off",
                                         "Too high workload on a leg", "Too small height for step", "Leg(s) can't reach destination point"
                                        };
  return ErrorMessages[errno];
}

void TSpider::SetErrno(TErrno error) {
  errno = error;
  BasicPosition();
  tasksQueue.Clear();
  esp.Clear();
}

void TSpider::Update_OnSurface_Worklods_Position() {
  if (!UpdateWorkloads()) {
    unsigned int amount = 0;
    for (int i = 0; i < 6; ++i)
      amount += legs[i].workload;
    onSurface = amount > minWorkloadOnSurface;
    board.UpdatePosition();
  }
}
