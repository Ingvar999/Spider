#include "Gyro.h"
#include "TSpider.h"
#include "Constants.h"

TSpider Spider1;

void setup()
{
  Spider1.Init(0, 0, 39, 6, 9, -5, -2, 12);
  Spider1.Init(1, 60, 31, 7, 8, -3, -2);
  Spider1.Init(2, 120, 30, 4, 10, -1, -2);
  Spider1.Init(3, 180, 34, 3, 11, -1, 8);
  Spider1.Init(4, -120, 35, 2, 12, 0, 8);
  Spider1.Init(5, -60, 38, 5, 13, -4, -3, 7);
  Serial.begin(250000);
  Serial1.begin(9600);
  Serial2.begin(250000);
  CalibrationGyro();
  Spider1.UpdateAllAngles();
}

void loop()
{
  CheckGyro();
  Spider1.CheckBalance();
  byte temp = 0;
  if (SerialX.available() > 0)
  {
    switch ((char)SerialX.read())
    {
      case 'd':
        SerialX.println(Spider1.SetRadius(SerialX.readStringUntil('.').toInt()));
        break;
      case 'w':
        Spider1.ChangeHeight(SerialX.readStringUntil('.').toInt());
        break;
      case 'r':
        temp = Spider1.ReadContacts();
        for (int i = 0; i < 6; ++i)
          SerialX.print((temp & bit(i)) != 0);
        SerialX.println();
        break;
      case 'g':
        SerialX.println(Spider1.toContacts());
        break;
      case 't':
        SerialX.println(Spider1.FixedTurn(SerialX.readStringUntil('.').toInt()));
        break;
      case 'y':
        SerialX.println(Spider1.Turn(SerialX.readStringUntil('.').toInt()));
        break;
      case 'b':
        Spider1.balancing = !Spider1.balancing;
        break;
      case 'z':
        Spider1.BasicPosition();
        break;
      case 'm':
        SerialX.println(Spider1.Move(SerialX.readStringUntil('.').toInt()));
        break;
      case 'v':
        SerialX.println(readVcc());
        break;
    }
  }
}

long readVcc()
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

  Delay(75);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  uint8_t low  = ADCL;
  uint8_t high = ADCH;
  long result = (high << 8) | low;
  result = 1125300L / result;
  return result;
}
