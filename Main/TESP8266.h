#pragma once

struct Request{
  byte id;
  char requestType;
  String requiredValues;
  char command_property;
  int argc;
  int args[2];
};

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
    void SendResponse(String); 
  private:
    HardwareSerial *espSerial;

    void ParseRequest(String data);
    void SendData(byte id, String data);
};
