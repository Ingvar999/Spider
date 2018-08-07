#pragma once
#ifndef _TSubBoard_h_
#define _TSubBoard_h_

#include "AngelsStruct.h"

class TSubBoard
{
  public:
    struct Angles position;

    void Init(HadwareSerial *);
    uint32_t GetVcc();
    void UpdatePosition();
    void TurnLegs(byte [7]);

  private:
    HardwareSerial *port;
};

#endif
