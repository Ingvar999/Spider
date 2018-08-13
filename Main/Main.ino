#include <TimerThree.h>
#include "TSpider.h"
#include "Constants.h"

TSpider Spider;

void Interrupt() {
  sei();
  Spider.CheckVcc();
  Spider.CheckBalance();
  Spider.WorkloadsAlignment();
}

void setup()
{
  Spider.Init();
  Spider.InitLeg(0, 0, 0, 6, 9, -5, -2, 12);
  Spider.InitLeg(1, 60, 3, 7, 8, -3, -2);
  Spider.InitLeg(2, 120, 4, 4, 10, -1, -2);
  Spider.InitLeg(3, 180, 5, 3, 11, -1, 8);
  Spider.InitLeg(4, -120, 1, 2, 12, 0, 8);
  Spider.InitLeg(5, -60, 2, 5, 13, -4, -3, 7);

  Serial.begin(250000);
  
  Spider.esp.Init(&Serial1);
  Spider.board.Init(&Serial2);
  Spider.UpdateAllAngles();

  Timer3.initialize(500000);
  Timer3.attachInterrupt(Interrupt);
  Timer3.start();

  Spider.PowerOn();
}

void loop()
{ 
  if (Spider.esp.ReadRequest()){
    String result = Spider.HandleCurrentRequest();
    Spider.esp.SendResponse(result);
  }
}


