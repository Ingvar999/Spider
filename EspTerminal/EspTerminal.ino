void setup()
{
  Serial.begin(250000);
  Serial1.begin(115200);
  Serial1.println("AT+CIPMUX=1");
  delay(500);
  Serial1.println("AT+CIPSERVER=1,80");
}

void loop()
{
  if (Serial.available() > 0)
  {
    Serial1.println(Serial.readString());
  }
  if (Serial1.available() > 0)
  {
    Serial.println(Serial1.readStringUntil('\n'));
  }
}
