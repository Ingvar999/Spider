#include "TESP8266.h"

void TESP8266::Init(HardwareSerial *port) {
  espSerial = port;
  espSerial->begin(115200);
  espSerial->println("AT+CIPMUX=1");
  espSerial->find("OK");
  espSerial->println("AT+CIPSERVER=1,80");
  espSerial->find("OK");
  espSerial->readString();
}

bool TESP8266::ReadRequest() {
  if (espSerial->available() > 0) {
    String data = espSerial->readStringUntil('\n');
    while ((espSerial->available() > 0) && !data.startsWith("+IPD"))
      data = espSerial->readStringUntil('\n');
    if (data.startsWith("+IPD")) {
      lastRequestId = data[5] - '0';
      ParseRequest(data);
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

void TESP8266::ParseRequest(String data) {
  if (data[7] == INFO || data[7] == DO) {
    currentRequest.RequestType = data[7];
    switch (currentRequest.RequestType) {
      case INFO:

        break;
      case DO:

        break;
    }
  }
  else
    currentRequest.RequestType = ERR;
}

char TESP8266::ReadCommand(int args[])
{
  char command = 0;
  String input = espSerial->readStringUntil('\n');
  while (!input.startsWith("+IPD,"))
    input = espSerial->readStringUntil('\n');
  Serial.println(input);
  lastId =  input[5] - '0';
  if (input.indexOf("HTTP") == -1) {
    isHttp = false;
    // just TCP
  }
  else {
    isHttp = true;
    if (input.indexOf("POST") == -1)
      return 0;
    else
      while (!input.startsWith("command="))
        Serial.println(input = espSerial->readStringUntil('\n'));
  }
  switch (command = input[input.indexOf('=') + 1]) { // parcing
    case 'r': case 'g': case 'b': case 'z': case 'v':
      return command;
      break;
    case 'd': case 'w': case 't': case 'y': case 'm':
      args[0] = input.substring(input.indexOf('+') + 1, input.indexOf('&')).toInt();
      return command;
    default:
      return 0;
      break;
  }
}

int TESP8266::SendData(int id, String *data) {
  espSerial->println("AT+CIPSEND=" + String(id) + "," + String(data->length()));
  espSerial->find(">");
  espSerial->print(*data);
  espSerial->find("OK");
  if (isHttp) {
    espSerial->println("AT+CIPCLOSE=" + String(id));
    espSerial->find("OK");
  }
  espSerial->readString();
}































