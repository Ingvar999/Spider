void setup()
{
 Serial.begin(115200);
 Serial1.begin(115200);
}

void loop()
{
  if (Serial.available()>0)
  {
    Serial1.println(Serial.readString());
    
  }
}
