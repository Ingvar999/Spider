#include "Constants.h"
#include "TESP8266.h"

void TESP8266::Init() {
  SerialX.begin(115200);
}

char TESP8266::ReadCommand(int args[])
{
  return 0;
}

int TESP8266::SendData(int id, String data) {
  String answer("");
  SerialX.println("AT+CIPSEND=" + String(id) + "," + answer.length());
  while (!Serial1.find(">"))
    delay(1);
  Serial.println(answer);
  SerialX.print(answer);
  while (!Serial1.find("OK"))
    delay(1);
  SerialX.println("AT+CIPCLOSE=" + String(id));
}































