#pragma once

#include "AnglesStruct.h"

class TSubBoard
{
  public:
    struct Angles position;
    uint16_t Vcc;

    void Init(HardwareSerial *);
    int UpdatePositionAndVcc();
    void TurnLegs(byte [7]);

  private:
    HardwareSerial *port;
};
