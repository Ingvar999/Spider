#include <TimerThree.h>

//#include <MsTimer2.h>

#include "TSpider.h"
#include "Constants.h"

TSpider Spider;

void Read() {
  sei();
  unsigned int amount[6] = {0};
  unsigned long t = millis();
  for (int i = 0; i < 30; ++i) {
    amount[0] += analogRead(0);
    amount[1] += analogRead(1);
    amount[2] += analogRead(2);
    amount[3] += analogRead(3);
    amount[4] += analogRead(4);
    amount[5] += analogRead(5);
  }
  Serial.println(millis() - t);
  for (int i = 0; i < 6; ++i){
    Serial.print(amount[i] / 30);
    Serial.print(' ');
  }
  Serial.println();
}

void setup()
{
  //digitalWrite(40, HIGH);
  Spider.Init(0, 0, 39, 6, 9, -5, -2, 12);
  Spider.Init(1, 60, 31, 7, 8, -3, -2);
  Spider.Init(2, 120, 30, 4, 10, -1, -2);
  Spider.Init(3, 180, 34, 3, 11, -1, 8);
  Spider.Init(4, -120, 35, 2, 12, 0, 8);
  Spider.Init(5, -60, 38, 5, 13, -4, -3, 7);
  analogReference(INTERNAL1V1);
  Serial.begin(250000);
  Serial2.begin(250000);
  Spider.esp.Init(&Serial1);
  Spider.UpdateAllAngles();


  Timer3.initialize(100000);
  Timer3.attachInterrupt(Read);
  Timer3.start();
  //MsTimer2::set(500, Read);
  //MsTimer2::start();
}

void loop()
{
  //delay(100);


  //Spider.CheckVcc();
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
}


