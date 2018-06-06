#pragma once
#ifndef _TESP8266_h_
#define _TESP8266_h_

class TESP8266
{
  public:
    bool isHttp;
  
    void Init(HardwareSerial *);
    bool hasData(){
      return (espSerial->available() > 0);
    }
    void Clear(){
      espSerial->readString();
    }
    char ReadCommand(int []);
    void SendHttpAnswer(String info);
    void SendTcpAnswer(long res);
  private:
    HardwareSerial *espSerial;
    byte lastId;

    int SendData(int id, String *data);
};
#endif
