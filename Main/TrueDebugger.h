#pragma once

#include "BasicDebugger.h"

class TrueDebugger: public BasicDebugger {
  public:
    TrueDebugger(TESP8266 *esp): BasicDebugger(esp) {
      Clear();
    }
    void Debug(const String msg)override {
      buffer += msg + "\n";
    }
    void Send(byte targetId)override {
      if (buffer.length() > 0) {
        transmitter->SendData(targetId, buffer);
        Clear();
      }
    }
    void Clear()override {
      buffer = "";
    }
};
