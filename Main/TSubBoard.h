#pragma once

#include "AnglesStruct.h"

class TSubBoard
{
  public:
    struct Angles position;

    void Init(HardwareSerial *);
    uint32_t GetVcc();
    void UpdatePosition();
    void TurnLegs(byte [7]);

  private:
    HardwareSerial *port;
};
