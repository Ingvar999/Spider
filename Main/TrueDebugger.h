#pragma once

class TrueDebugger{
  public:
    TrueDebugger(TESP8266 *esp): transmitter(esp) {
      Clear();
    }
    void Debug(const String msg) {
      buffer += msg + "\n";
    }
    void Send(byte targetId) {
      if (buffer.length() > 0) {
        buffer.remove(buffer.length() - 1, 1);
        transmitter->SendData(targetId, buffer);
        Clear();
      }
    }
    void Clear() {
      buffer = "";
    }
   protected:
    TESP8266 *transmitter;
    String buffer;
};
