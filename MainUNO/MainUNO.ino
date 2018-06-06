#include <Servo.h>
#include <MsTimer2.h>
#include "TGyro.h"

TGyro gyro;
Servo myServo[6];
byte val[6];
byte newVal[7]  = {90, 90, 90, 90, 90, 90, 20};
short pog[6] = {10, 5, -3, -3, -8, 3};
byte pins[6] = {5, 3, 11, 10, 9, 6};

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
  Serial.setTimeout(500);
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
    switch ((char)Serial.read())
    {
      case 'w':
        Serial.readBytes(newVal, 7);
        UpdateAllAngles();
        break;
      case 'r':
        Serial.write((byte *)&gyro.angels, sizeof(struct Angels));
        break;
    }
  }
}
