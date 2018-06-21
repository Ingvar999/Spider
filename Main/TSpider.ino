#include "TSpider.h"
#include "Constants.h"
#include "AngelsStruct.h"

void TSpider::Init(int i, int _pos, int pinCont, int pin1, int pin2, int pog1 = 0, int pog2 = 0, int _qR = 10)
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
  Radius = 30;
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

int TSpider::MinHeight()
{
  int res = legs[0].GetHeight();
  for (int i = 1; i < 6; ++i)
    if (legs[i].GetHeight() < res)
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
        if (legs[i].GetHeight() < lifting)
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
      delay(2000);
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
        delay(100);
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

int TSpider::Balance(struct Angels *angels)
{
  int error = toContacts();
  if (error == 0)
  {
    //SerialX.print(horizontal);
    //SerialX.print(" / ");
    //SerialX.println(vertical);
    int dh = 0;
    float tanV = tan(angels->vertical * ToRad), tanPV = tan(positionV * ToRad);
    for (int i = 0; i < 6; ++i)
    {
      dh = round((Radius + a) * (cos((legs[i].GetPosition() - angels->horizontal) * ToRad) * tanV -
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
    struct Angels *angels = ReadGyro();
    if (balancing && ((abs(angels->vertical - positionV) > maxSkew ||
                       abs(angels->horizontal - positionH) > 4 * maxSkew && positionV != 0)))
      if (!GetContacts())
        balancing = false;
      else if (error = Balance(angels))
        balancing = false;
    lastBalancingTime = millis();
    delete angels;
  }
  return error;
}

int TSpider::Move(int direction)
{
  int error = toContacts();
  if (error == 0)
  {
    for (int i = 0; i < 6; ++i)
      if (legs[i].GetHeight() < lifting)
        return 3;
    byte Val[7] = {90, 90, 90, 90, 90, 90, 10};
    int i, a = 0;
    while (!esp.hasData())
    {
      ThreeLegsUpDown(a, a + 2, a + 4, -1);

      for (i = a; i < 6; i += 2)
        Val[i] = legs[i].ForStep(stepLength, direction, Radius);
      for (i = (a + 1) % 2; i < 6; i += 2)
        Val[i] = legs[i].ForStep(stepLength, 180 + direction, Radius);

      Serial2.write('w');
      Serial2.write(Val, 7);
      UpdateAllAngles();

      ThreeLegsUpDown(a, a + 2, a + 4, 1);
      toContacts();
      delay(50);
      a = (a + 1) % 2;
    }
    esp.Clear();
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
    delay(50);
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
inline void TSpider::TwoLegsUpDown(int i, int j, int dir)
{
  legs[i].SetHeight(legs[i].GetHeight() + dir * lifting);
  legs[j].SetHeight(legs[j].GetHeight() + dir * lifting);
  UpdateAllAngles();
}

inline void TSpider::ThreeLegsUpDown(int i, int j, int k, int dir)
{
  legs[i].SetHeight(legs[i].GetHeight() + dir * lifting);
  legs[j].SetHeight(legs[j].GetHeight() + dir * lifting);
  legs[k].SetHeight(legs[k].GetHeight() + dir * lifting);
  UpdateAllAngles();
}

long TSpider::ReadVcc()
{
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(50);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  uint8_t low  = ADCL;
  uint8_t high = ADCH;
  long result = (high << 8) | low;
  result = 1125300L / result;
  return result;
}

void TSpider::CheckVcc()
{
  if (ReadVcc() < 5000)
    BasicPosition();
}

String TSpider::GetInfoInHtml(long result) {
  struct Angels *angels = ReadGyro();
  String info = "<tr><td>Радиус: </td><td>" + String(Radius) + "</td></tr>"
    "<tr><td>Высота: </td><td>" + String(MinHeight()) + "</td></tr>"
    "<tr><td>Балансировка: </td><td>" + (balancing ? "Вкл." : "Выкл.") + "</td></tr>"
    "<tr><td>Модуль наклона: </td><td>" + String(angels->vertical) + "</td></tr>"
    "<tr><td>Направление наклона: </td><td>" + String(angels->horizontal) + "</td></tr>"
    "<tr><td>Напряжение: </td><td>" + String(ReadVcc()) + "</td></tr>"
    "<tr><td>Результат: </td><td>" + String(result) + "</td></tr>";
  delete angels;
  return info;
}

struct Angels *TSpider::ReadGyro() {
  Serial2.print('r');
  struct Angels *angels = new struct Angels;
  memset(angels, 0, sizeof(struct Angels));
  Serial2.readBytes((byte *)angels, sizeof(struct Angels));
  return angels;
}
































