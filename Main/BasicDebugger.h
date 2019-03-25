#pragma once

#include "TESP8266.h"

class BasicDebugger{
  public:
  BasicDebugger(TESP8266 *esp): transmitter(esp){}
  virtual void Debug(const String msg){}
  virtual void Send(byte targetId){}
  virtual void Clear(){}
  protected:
  TESP8266 *transmitter;
  String buffer;
};
