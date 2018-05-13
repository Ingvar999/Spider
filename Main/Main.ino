#include "TSpider.h"
#include "Constants.h"

TSpider Spider;

void setup()
{
  Spider.Init(0, 0, 39, 6, 9, -5, -2, 12);
  Spider.Init(1, 60, 31, 7, 8, -3, -2);
  Spider.Init(2, 120, 30, 4, 10, -1, -2);
  Spider.Init(3, 180, 34, 3, 11, -1, 8);
  Spider.Init(4, -120, 35, 2, 12, 0, 8);
  Spider.Init(5, -60, 38, 5, 13, -4, -3, 7);
  Serial.begin(250000);
  Serial2.begin(250000);
  Spider.esp.Init();
  Spider.gyro.CalibrationGyro();
  Spider.UpdateAllAngles();
}

void loop()
{
  Spider.CheckVcc();
  Spider.gyro.CheckGyro();
  Spider.CheckBalance();
  if (SerialX.available() > 0)
  {
    int args[2];
    switch (Spider.esp.ReadCommand(args))
    {
      case 'd':
        SerialX.println(Spider.SetRadius(SerialX.readStringUntil('.').toInt()));
        break;
      case 'w':
        Spider.ChangeHeight(SerialX.readStringUntil('.').toInt());
        break;
      case 'r': {
          byte temp = Spider.ReadContacts();
          for (int i = 0; i < 6; ++i)
            SerialX.print((temp & bit(i)) != 0);
          SerialX.println();
        }
        break;
      case 'g':
        SerialX.println(Spider.toContacts());
        break;
      case 't':
        SerialX.println(Spider.FixedTurn(SerialX.readStringUntil('.').toInt()));
        break;
      case 'y':
        SerialX.println(Spider.Turn(SerialX.readStringUntil('.').toInt()));
        break;
      case 'b':
        Spider.balancing = !Spider.balancing;
        break;
      case 'z':
        Spider.BasicPosition();
        break;
      case 'm':
        SerialX.println(Spider.Move(SerialX.readStringUntil('.').toInt()));
        break;
      case 'v':
        SerialX.println(Spider.ReadVcc());
        break;
    }
  }
}


