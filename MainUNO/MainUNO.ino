#include <Servo.h>
#include <MsTimer2.h>
#include "TGyro.h"

TGyro gyro;
Servo myServo[6];

byte val[6];
byte newVal[7]  = {90, 90, 90, 90, 90, 90, 20};
short pog[6] = {10, 5, -3, -3, -8, 3};
byte pins[6] = {5, 3, 11, 10, 9, 6};
uint32_t currentVcc;

uint32_t ReadVcc()
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

  delay(30);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  uint8_t low  = ADCL;
  uint8_t high = ADCH;
  uint32_t result = (high << 8) | low;
  result = 1125300UL / result;
  return result;
}

void UpdateAllAngles()
{
  int done = 1, i = 0;
  do
  {
    done = 1;
    for (i = 0; i < 6; i++)
      if (newVal[i] != val[i])
      {
        if (newVal[i] > val[i])
          ++val[i];
        else
          --val[i];
        myServo[i].write(val[i] + pog[i]);
        done = 0;
      }
    delay(newVal[6]);
  } while (!done);
}

void Interrupt(){
  sei();
  gyro.UpdateGyro();
}

void setup() {
  Serial.begin(250000);
  
  gyro.CalibrationGyro();
  
  MsTimer2::set(100, Interrupt);
  MsTimer2::start();

  for (int i = 0; i < 6; ++i){
    myServo[i].attach(pins[i]);
    val[i] = myServo[i].read();
  }
  UpdateAllAngles();
}

void loop()
{
  if (Serial.available() > 0)
  {
    MsTimer2::stop();
    switch ((char)Serial.read())
    {
      case 'w':
        Serial.readBytes(newVal, 7);
        UpdateAllAngles();
        break;
      case 'r':
        //gyro.UpdateGyro();
        Serial.write((byte *)&gyro.angels, sizeof(struct Angels));
        break;
      case 'v':
        currentVcc = ((uint32_t)analogRead(0) * 10000) >> 10;
        Serial.write((byte *)&currentVcc, sizeof(currentVcc));
        break;
    }
    MsTimer2::start();
  }
}
