#pragma once
#ifndef _TESP8266_h_
#define _TESP8266_h_

struct Request{
  char requestType;
  String requiredValues;
  char command;
  int args[2];
}

class TESP8266
{
  public:
    static const char INFO = 'i';
    static const char DO = 'd';
    static const char ERR = 0;

    Request currentRequest;
  
    void Init(HardwareSerial *);
    bool ReadRequest();
  private:
    HardwareSerial *espSerial;
    byte lastRequestId;

    void ParseRequest(String data);
    int SendData(String data);
};
#endif
