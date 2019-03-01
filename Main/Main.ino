#include "TSpider.h"

void setup()
{
  Serial.begin(250000);
  Spider.Init();
  Spider.InitLeg(0, 0, 0, 6, 9, -5, -2, 12);
  Spider.InitLeg(1, 60, 3, 7, 8, -3, -2);
  Spider.InitLeg(2, 120, 4, 4, 10, -1, -2);
  Spider.InitLeg(3, 180, 5, 3, 11, -1, 8);
  Spider.InitLeg(4, -120, 1, 2, 12, 0, 8);
  Spider.InitLeg(5, -60, 2, 5, 13, -4, -3, 7);
  
  Spider.esp.Init(&Serial1);
  Spider.board.Init(&Serial2);
  Spider.UpdateAllAngles();
  Spider.StartTimer(500);
}

void loop()
{
  Spider.DoCommands();
}
