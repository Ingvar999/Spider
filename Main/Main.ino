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
  Spider.esp.Init(&Serial1);
  Spider.UpdateAllAngles();
}

void loop()
{
  //unsigned long t = millis();
  Spider.CheckVcc();
  Spider.CheckBalance();
  if (Spider.esp.hasData())
  {
    int args[2];
    long result = 0;
    switch (Spider.esp.ReadCommand(args))
    {
      case 'd':
        result = Spider.SetRadius(args[0]);
        break;
      case 'w':
        result = Spider.ChangeHeight(args[0]);
        break;
      case 'r': 
        result = Spider.ReadContacts();
        break;
      case 'g':
        result = Spider.toContacts();
        break;
      case 't':
        result = Spider.FixedTurn(args[0]);
        break;
      case 'y':
        result = Spider.Turn(args[0]);
        break;
      case 'b':
        Spider.balancing = !Spider.balancing;
        break;
      case 'z':
        Spider.BasicPosition();
        break;
      case 'm':
        result = Spider.Move(args[0]);
        break;
      case 'v':
        result = Spider.ReadVcc();
        break;
      default:
        result = 9;
        break;
    }
    if (Spider.esp.isHttp)
      Spider.esp.SendHttpAnswer(Spider.GetInfoInHtml(result));
    else
      Spider.esp.SendTcpAnswer(result);
  }
  //Serial.println(millis() - t);
}


