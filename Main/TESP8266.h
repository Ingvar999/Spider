#pragma once
#ifndef _TESP8266_h_
#define _TESP8266_h_

class TESP8266
{
  public:
    void Init();
    char ReadCommand(int []);
    int SendData(int id, String data);
  private:

};
#endif
