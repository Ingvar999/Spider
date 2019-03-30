#pragma once

#include "TESP8266.h"

class FakeDebugger{
  public:
  FakeDebugger(TESP8266 *esp){}
  void Debug(const String msg){}
  void Send(byte targetId){}
  void Clear(){}
};
