#include "TESP8266.h"

const String pageHtml = "<!DOCTYPE HTML>" 
"<html>" 
"<head>"
"<title>Spider</title>" 
"</head>" 
"<body>"
"<form method=post>"
"<table style=\"margin: 50px auto;\">"
"{info}"
"<tr><td><br><input type=text name=command></td>"
"<td><br><input type=submit name=send value=Отправить autofocus></td></tr>"
"</table>"
"</form>"
"</body>"
"</html>";

const String positiveHeader = "HTTP/1.1 200 OK\r\n"
"Server: ESP8266\r\n"
"Content-Type: text/html; charset=UTF-8\r\n"
"Content-Length: {size}\r\n"
"Connection: Closed\r\n\r\n{data}";

void TESP8266::Init(HardwareSerial *port) {
  espSerial = port;
  espSerial->begin(115200);
  espSerial->println("AT+CIPMUX=1");
  espSerial->find("OK");
  espSerial->println("AT+CIPSERVER=1,80");
  espSerial->find("OK");
  espSerial->readString();
}

char TESP8266::ReadCommand(int args[])
{
  char command = 0;
  String input = espSerial->readStringUntil('\n');
  while (!input.startsWith("+IPD,"))
    input = espSerial->readStringUntil('\n');
  Serial.println(input);
  lastId =  input[5] - '0';
  if (input.indexOf("HTTP") == -1){
    isHttp = false;
    // just TCP
  }
  else{
    isHttp = true;
    if (input.indexOf("POST") == -1)
      return 0;
    else
      while (!input.startsWith("command="))
        input = espSerial->readStringUntil('\n');
  }
  switch (command = input[input.indexOf('=')+1]){ // parcing
    case 'r': case 'g': case 'b': case 'z': case 'v':
      return command;
      break;
    case 'd': case 'w': case 't': case 'y': case 'm':
      args[0] = input.substring(input.indexOf('+')+1, input.indexOf('&')).toInt();
      return command;
    default:
      return 0;
      break;
  }
}

void TESP8266::SendHttpAnswer(String info){
  String page = pageHtml;
  page.replace("{info}", info);
  String data = positiveHeader;
  data.replace("{size}", String(page.length()));
  data.replace("{data}", page);
  SendData(lastId, &data);
}

void TESP8266::SendTcpAnswer(long result){
  
}

int TESP8266::SendData(int id, String *data) {
  espSerial->println("AT+CIPSEND=" + String(id) + "," + String(data->length()));
  espSerial->find(">");
  espSerial->print(*data);
  espSerial->find("OK");
  if (isHttp){
    espSerial->println("AT+CIPCLOSE=" + String(id));
    espSerial->find("OK");
  }
  espSerial->readString();
}































