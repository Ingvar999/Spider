#pragma once

#include "Request.h"

class TESP8266
{
  public:
    static const char INFO = 'i';
    static const char DO = 'd';
    static const char SET = 's';
    static const char ERR = 0;

    Request currentRequest;

    inline bool HasData();
    inline void Clear();
    void Init(HardwareSerial *);
    bool ReadRequest();
    void SendResponse(const String&);
    void SendData(const byte id, const String &data); 
  private:
    HardwareSerial *espSerial;

    void ParseRequest(const String &data);
};
