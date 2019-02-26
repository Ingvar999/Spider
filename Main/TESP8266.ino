#include "TESP8266.h"

inline bool TESP8266::HasData(){
  return espSerial->available();
}

inline void TESP8266::Clear(){
  espSerial->readString();
}

void TESP8266::Init(HardwareSerial *port) {
  espSerial = port;
  espSerial->begin(115200);
  espSerial->println("AT+CIPMUX=1");
  espSerial->find("OK");
  espSerial->println("AT+CIPSERVER=1,80");
  espSerial->find("OK");
  Clear();
}

bool TESP8266::ReadRequest() {
  if (espSerial->available() > 0) {
    String data = espSerial->readStringUntil('\n');
    while ((espSerial->available() > 0) && !data.startsWith("+IPD")){
      data = espSerial->readStringUntil('\n');
    }
    if (data.startsWith("+IPD")) {
      currentRequest.id = data[5] - '0';
      ParseRequest(data.substring(data.indexOf(':')+1));
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

void TESP8266::ParseRequest(String data) {
  if (data.length() >= 2 && (data[0] == INFO || data[0] == DO || data[0] == SET)) {
    currentRequest.requestType = data[0];
    switch (currentRequest.requestType) {
      case INFO:
          currentRequest.requiredValues = data.substring(1);
        break;
      case DO: case SET:
        currentRequest.command_property = data[1];
        if (data.length() > 2){
          int spacePos = data.indexOf(' ');
          if (spacePos == -1){
            currentRequest.args[0] = data.substring(2).toInt();
            currentRequest.argc = 1;
          }
          else{
            currentRequest.args[0] = data.substring(2, spacePos).toInt();
            currentRequest.args[1] = data.substring(spacePos + 1).toInt();
            currentRequest.argc = 2;
          }
        }
        else {
          currentRequest.argc = 0;
        }
        break;
    }
  }
  else
    currentRequest.requestType = ERR;
}

void TESP8266::SendResponse(String result){
  SendData(currentRequest.id, result);
}

void TESP8266::SendData(byte id, String data) {
  espSerial->println("AT+CIPSEND=" + String(id) + "," + String(data.length()));
  espSerial->find(">");
  espSerial->print(data);
  espSerial->find("OK");
}
