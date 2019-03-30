#include <TimerThree.h>
#include "TSpider.h"
#include "Constants.h"

template<typename T>
void TSpider<T>::ControlServices() {
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

template<typename T>
inline void TSpider<T>::PowerOn() {
  digitalWrite(powerPin, HIGH);
  powerOn = true;
}

template<typename T>
inline void TSpider<T>::PowerOff() {
  digitalWrite(powerPin, LOW);
  powerOn = false;
}

template<typename T>
void TSpider<T>::Init() {
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);

  pinMode(lightDetectionPin, INPUT);
  pinMode(ledPin, OUTPUT);
  analogReference(INTERNAL1V1);

  sonar = new NewPing(trigPin, echoPin, 150);
  debugger = new T(&esp);
  Spider.esp.Init(&Serial1);
  Spider.board.Init(&Serial2);
}

template<typename T>
void TSpider<T>::StartTimer(unsigned long miliseconds) {
  Timer3.initialize(miliseconds * 1000);
  Timer3.attachInterrupt(TimerHandler);
  Timer3.start();
}

template<typename T>
void TSpider<T>::InitLeg(int i, int _pos, int pinCont, int pin1, int pin2, int pog1 = 0, int pog2 = 0, int _qR = 10)
{
  legs[i].Init(_pos, pinCont, pin1, pin2, pog1, pog2, _qR);
  legs[i].R = Radius;
}

template<typename T>
void TSpider<T>::UpdateAllAngles()
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

template<typename T>
//второй параметр используется при контроле высоты
void TSpider<T>::ChangeHeight(int delta, bool changeProperty = true)
{
  int error = 0;
  Timer3.stop();
  if (changeProperty && !onSurface) {
    ReachGround();
    if (errno == OK) {
      height = MinHeight();
    }
  }
  if (errno == OK) {
    int i;
    for (i = 0; i < 6 && !error; ++i)
      error = legs[i].ChangeHeight(delta);
    if (error) {
      debugger->Debug(String(onSurface) + " " + String(changeProperty) + " " + String(height) + " " + String(delta) + " " + String(legs[i].GetHeight()));
      debugger->Debug("ChangeHeight sets errno");
      SetErrno(LEG_CANNOT_REACH_POINT);
    }
    else if (changeProperty) {
      height += delta;
      UpdateAllAngles();
      delay(stepDelaying);
    }
  }
  Timer3.start();
}

template<typename T>
void TSpider<T>::BasicPosition()
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

template<typename T>
int TSpider<T>::MaxHeight()
{
  int res = legs[0].GetHeight();
  for (int i = 1; i < 6; ++i)
    if (legs[i].GetHeight() > res)
      res = legs[i].GetHeight();
  return res;
}

template<typename T>
int TSpider<T>::MinHeight()
{
  int res = legs[0].GetHeight();
  for (int i = 1; i < 6; ++i)
    if (legs[i].GetHeight() < res)
      res = legs[i].GetHeight();
  return res;
}

template<typename T>
void TSpider<T>::SetRadius(int newR)
{
  if ((newR >= minRadius) && (sqrL1L2 > sqr(MaxHeight()) + sqr(newR)))
  {
    if (!onSurface)
    {
      Timer3.stop();
      for (int i = 0; i < 6; ++i)
        legs[i].R = newR;
      UpdateAllAngles();
      Radius = newR;
      delay(stepDelaying);
      Timer3.start();
    }
    else {
      if (height >= minLifting) {
        Timer3.stop();
        Radius = newR;
        for (int i = 0; i < 2 && errno == OK; i++)
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
      }
      else {
        debugger->Debug("SetRadius sets errno");
        SetErrno(TOO_SMALL_HEIGHT);
      }
    }
  }
  else {
    debugger->Debug("SetRadius sets errno");
    SetErrno(LEG_CANNOT_REACH_POINT);
  }
}

template<typename T>
void TSpider<T>::Turn(int angle)
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
  }
  else {
    debugger->Debug("Turn sets errno");
    SetErrno(LEG_CANNOT_REACH_POINT);
  }
}

template<typename T>
void TSpider<T>::FixedTurn(int angle)
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
      while (steps-- && tasksQueue.isEmpty() && errno == OK) {
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

        board.TurnLegs(values);
        UpdateAllAngles();
        delay(stepDelaying);
        ControlServices();
      }
      delay(stepDelaying);
      Timer3.start();
    }
    else {
      debugger->Debug("FixedTurn sets errno");
      SetErrno(LEG_CANNOT_REACH_POINT);
    }
  }
  else {
    debugger->Debug("FixedTurn sets errno");
    SetErrno(TOO_SMALL_HEIGHT);
  }
}

template<typename T>
void TSpider<T>::Move(int direction, bool wander)
{
  if (height >= minLifting) {
    byte values[7] = {90, 90, 90, 90, 90, 90, motionDelaying};
    int i, a = 0;
    Timer3.stop();
    while (tasksQueue.isEmpty() && errno == OK) {
      while (wander && !isValidDistance() && errno == OK) {
        FixedTurn(50);
      }
      while ((!wander || wander && isValidDistance()) && errno == OK && tasksQueue.isEmpty()) {
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
        if (errno == OK) {
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
      }
    }
    delay(stepDelaying);
    Timer3.start();
  }
  else {
    debugger->Debug("Move sets errno");
    SetErrno(TOO_SMALL_HEIGHT);
  }
}

template<typename T>
inline void TSpider<T>::TwoLegsUpDown(int i, int j, int dir)
{
  legs[i].ChangeHeight(dir * height);
  legs[j].ChangeHeight(dir * height);
  UpdateAllAngles();
}

template<typename T>
inline void TSpider<T>::ThreeLegsUpDown(int i, int j, int k, int dir)
{
  legs[i].ChangeHeight(dir * height);
  TwoLegsUpDown(j, k, dir);
}

template<typename T>
void TSpider<T>::CheckVcc()
{
  if (checkVccActive)
    if (powerOn) {
      if (board.GetVcc() < minVoltage) {
        debugger->Debug("CheckVcc sets errno");
        SetErrno(LOW_BATTARY);
        PowerOff();
      }
    }
}

template<typename T>
int TSpider<T>::Balance()
{
  int dh = 0, error = 0, i;
  float tanV = tan(board.position.vertical * ToRad), tanPV = tan(positionV * ToRad);
  for (i = 0; i < 6 && !error; ++i)
  {
    dh = round((Radius + a) * (cos((legs[i].GetPosition() - board.position.horizontal) * ToRad) * tanV -
                               cos((legs[i].GetPosition() - positionH) * ToRad) * tanPV));
    error = legs[i].ChangeHeight(dh);
  }
  
  String deb = "Balance " + String(legs[0].GetHeight());
  for (int i = 1; i < 6; ++i) {
    deb += " " + String(legs[i].GetHeight());
  }
  debugger->Debug(deb);
  debugger->Debug(String(board.position.vertical) + " " + String(board.position.horizontal));
  if (error) {
    debugger->Debug("Balance sets errno");
    debugger->Debug(String(onSurface) + " " + String(height) + " " + String(dh) + " " + String(board.position.vertical) + " " + String(legs[i].GetHeight()));
    SetErrno(LEG_CANNOT_REACH_POINT);
  }
  return error;
}

template<typename T>
int TSpider<T>::PositionAlignment()
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

template<typename T>
int TSpider<T>::UpdateWorkloads() {
  unsigned long amount[6] = {0};
  for (int i = 0; i < 30; ++i)
    for (int j = 0; j < 6; ++j)
      amount[j] += legs[j].ReadVoltage();

  for (int i = 0; i < 6; ++i) {
    legs[i].workload = amount[i] / 30;
    if (legs[i].workload > maxWorkloadThreshold) {
      debugger->Debug("UpdateWorkload sets errno");
      SetErrno(HIGH_WORKLOAD);
      return 1;
    }
  }
  return 0;
}

template<typename T>
void TSpider<T>::ReachGround() {
  int error = 0;
  bool done;
  do {
    UpdateWorkloads();
    if (errno == OK) {
      done = true;
      for (int i = 0; i < 6 && !error; ++i) {
        if (legs[i].workload < minWorkloadThreshold) {
          done = false;
          error = legs[i].ChangeHeight(4);
        }
      }
      if (!done) {
        UpdateAllAngles();
        delay(60);
      }
    }
  } while ( !error && !done && errno == OK);
  if (error) {
    debugger->Debug("ReachGround sets errno");
    SetErrno(LEG_CANNOT_REACH_POINT);
  }
  return error;
}

template<typename T>
int TSpider<T>::HeightControl() {
  if (heightControlActive) {
    int realHeight = MinHeight();
    if (height != realHeight) {
      ChangeHeight(height - realHeight, false);
      String deb = "HControl " + String(legs[0].GetHeight());
      for (int i = 1; i < 6; ++i) {
        deb += " " + String(legs[i].GetHeight());
      }
      debugger->Debug(deb);
      return errno;
    }
  }
  return 0;
}

template<typename T>
int TSpider<T>::WorkloadsAlignment() {
  if (workloadsAlignemtActive) {
    unsigned int amount = 0;
    int error = 0;
    for (int i = 0; i < 6; ++i)
      amount += legs[i].workload;
    int avarageWorkload = amount / 6;
    for (int i = 0; i < 6 && !error; ++i) {
      error = legs[i].ChangeHeight((int)((float)(avarageWorkload - legs[i].workload) / (maxWorkloadDisparityRate * avarageWorkload)));
    }
    String deb = "Alignment " + String(legs[0].GetHeight());
    for (int i = 1; i < 6; ++i) {
      deb += " " + String(legs[i].GetHeight());
    }
    debugger->Debug(deb);
    if (error) {
      debugger->Debug("WorkloadsAlignment sets errno");
      SetErrno(LEG_CANNOT_REACH_POINT);
    }
    return error;
  }
  return 0;
}

template<typename T>
String TSpider<T>::HandleCurrentRequest() {
  switch (esp.currentRequest.requestType) {
    case TESP8266::DO: {
        if (errno != POWER_OFF && errno != LOW_BATTARY) {
          if (!tasksQueue.isFull()) {
            TTask task(esp.currentRequest);
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
      }
      break;
    case TESP8266::INFO:
      return GetInfo();
      break;
    case TESP8266::SET:
      return SetProperty();
      break;
    case TESP8266::ERR:
      return "Invalid request type";
      break;
  }
}

template<typename T>
void TSpider<T>::DispatchTasksQueue() {
  if (!tasksQueue.isEmpty()) {
    const TTask task = tasksQueue.Pop();
    switch (task.argc) {
      case 0: {
          switch (task.command)
          {
            case 'b': {
                BasicPosition();
              }
              break;
            case 'm': {
                Move(0, true);
              }
              break;
            case 's': {
              }
              break;
            default:
              debugger->Debug("Invalid command for 0 parameters");
              break;
          }
        }
        break;
      case 1: {
          switch (task.command)
          {
            case 'r': {
                SetRadius(task.args[0]);
              }
              break;
            case 'h': {
                ChangeHeight(task.args[0]);
              }
              break;
            case 'f': {
                FixedTurn(task.args[0]);
              }
              break;
            case 't': {
                Turn(task.args[0]);
              }
              break;
            case 'm': {
                Move(task.args[0], false);
              }
              break;
            default:
              debugger->Debug("Invalid command for 1 parameter");
              break;
          }
        }
        break;
      case 2: {
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
            default:
              debugger->Debug("Invalid command for 2 parameters");
              break;
          }
        }
        break;
    }
    debugger->Send(task.sender);
  }
}

template<typename T>
String TSpider<T>::GetInfo() {
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
        result += String(balanceActive) + String(workloadsAlignemtActive) +
                  String(heightControlActive) + String(checkVccActive) + String(lightControlActive) +
                  String(isLightning) + String(powerOn);
        break;
      case 's':
        result += String(maxMotionDelaying - motionDelaying);
        break;
      case 'c':
        result += String(minDistance);
        break;
      case 'q':
        result += String(positionV) + " " + String(positionH);
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

template<typename T>
String TSpider<T>::SetProperty() {
  switch (esp.currentRequest.command_property)
  {
    case 'i': {
        bool toOn = (bool)esp.currentRequest.args[0];
        if (toOn) {
          PowerOn();
          if (errno == POWER_OFF || errno == LOW_BATTARY) {
            errno = OK;
          }
        }
        else {
          SetErrno(POWER_OFF);
          PowerOff();
        }
      }
      break;
    case 'w': {
        workloadsAlignemtActive = esp.currentRequest.args[0];
      }
      break;
    case 'b': {
        balanceActive = esp.currentRequest.args[0];
      }
      break;
    case 'h': {
        heightControlActive = esp.currentRequest.args[0];
      }
      break;
    case 'c': {
        checkVccActive = esp.currentRequest.args[0];
        if (!checkVccActive && errno == LOW_BATTARY) {
          errno = POWER_OFF;
        }
      }
      break;
    case 'l': {
        lightControlActive = esp.currentRequest.args[0];
      }
      break;
    case 'f': {
        lightControlActive = false;
        TurnLight(esp.currentRequest.args[0]);
      }
      break;
    case 'd': {
        minDistance = esp.currentRequest.args[0];
      }
      break;
    case 's': {
        motionDelaying = maxMotionDelaying - esp.currentRequest.args[0];
      }
      break;
    case 'p': {
        positionV = esp.currentRequest.args[0];
        positionH = esp.currentRequest.args[1];
      }
      break;
    default: {
        return "Invalid property";
      }
      break;
  }
  return "Recieved";
}

template<typename T>
void TSpider<T>::CheckLight() {
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

template<typename T>
void TSpider<T>::TurnLight(int state) {
  isLightning = (bool)state;
  digitalWrite(ledPin, state);
}

template<typename T>
String TSpider<T>::GetErrorMessage() {
  static const String ErrorMessages[] = {"OK", "Problem with a subboard", "Low battary for moving", "Legs power is off",
                                         "Too high workload on a leg", "Too small height for step", "Leg(s) can't reach destination point"
                                        };
  return ErrorMessages[errno];
}

template<typename T>
void TSpider<T>::SetErrno(TErrno error) {
  errno = error;
  BasicPosition();
  tasksQueue.Clear();
  esp.Clear();
}

template<typename T>
void TSpider<T>::Update_OnSurface_Worklods_Position() {
  static const int minLegsOnSurface = 5;
  if (!UpdateWorkloads()) {
    int legsOnSurface = 0;
    for (int i = 0; i < 6; ++i) {
      if (legs[i].workload > minWorkloadThreshold) {
        ++legsOnSurface;
      }
    }
    onSurface = legsOnSurface >= minLegsOnSurface;
    board.UpdatePosition();
  }
}
