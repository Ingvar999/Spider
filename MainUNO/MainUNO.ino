#include <Servo.h>

Servo myServo[6];
byte val[6];
byte newVal[7]  = {90, 90, 90, 90, 90, 90, 20};
short pog[6] = {10, 5, -3, -3, -8, 3};

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

void setup() {
  Serial.begin(250000);
  Serial.setTimeout(1000);
  myServo[0].attach(5);
  myServo[1].attach(3);
  myServo[2].attach(11);
  myServo[3].attach(10);
  myServo[4].attach(9);
  myServo[5].attach(6);
  for (int i = 0; i < 6; ++i)
    val[i] = myServo[i].read();
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
    }
  }
}
