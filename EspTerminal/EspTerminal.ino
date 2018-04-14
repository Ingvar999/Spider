
String datapos = "<!DOCTYPE HTML>" 
"<html>" 
"<head>"
"<title>Arduinka</title>" 
"</head>" 
"<body>"
"<h1>It works!!! :D</h1>"
"</body>"
"</html>";

String positiveAnswer = "HTTP/1.1 200 OK\r\n"
"Server: esp8266\r\n"
"Content-Type: text/html; charset=UTF-8\r\n"
"Content-Length: "+ String(datapos.length()) + "\r\n"
//"Set-Cookie: sessionToken=abc123; Expires=Wed, 29 Mar 2018 00:00:00 GMT\r\n"
"Connection: Closed\r\n\r\n" + datapos;

String dataneg = "<!DOCTYPE HTML>" 
"<html>" 
"<head>"
"<title>404 Not Found</title>" 
"</head>" 
"<body>" 
"<h1>Not Found</h1>" 
"<p>The requested URL / was not found on this server.</p>" 
"</body>" 
"</html>";

String negativeAnswer = "HTTP/1.1 404 Not Found\r\n"
"Server: esp8266\r\n"
"Content-Type: text/html; charset=UTF-8\r\n"
"Content-Length: "+ String(dataneg.length()) + "\r\n"
"Connection: Closed\r\n\r\n" + dataneg;

void setup()
{
  Serial.begin(115200);
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
    String input = Serial1.readStringUntil('\n');
    if (input.startsWith("+IPD,"))
       {
          Serial.println(input);
          int id =  input[5] - '0';
          String answer;
          if (input.substring(input.indexOf('/'), input.lastIndexOf(' ')) == String("/"))
            answer = positiveAnswer;
          else
            answer = negativeAnswer;
          while (Serial1.available()>0)
            Serial.println(Serial1.readStringUntil('\n'));
          Serial1.println("AT+CIPSEND="+String(id)+","+answer.length());
          while (!Serial1.find(">"))
            delay(1);
          Serial.println(answer);
          Serial1.print(answer);
          while (!Serial1.find("OK"))
            delay(1);
          Serial1.println("AT+CIPCLOSE="+String(id));
       }
       else
    Serial.println(input);
  }
}
