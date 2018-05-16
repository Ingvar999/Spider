
String datapos = "<!DOCTYPE HTML>" 
"<html>" 
"<head>"
"<title>Spider</title>" 
"</head>" 
"<body>"
"<form method=post>"
"<input type=text name=command>"
"<input type=submit name=send value=Send>"
"</form>"
"</body>"
"</html>";

String positiveAnswer = "HTTP/1.1 200 OK\r\n"
"Server: ESP8266\r\n"
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
"<p>The requested URL was not found on this server.</p>" 
"</body>" 
"</html>";

String negativeAnswer = "HTTP/1.1 404 Not Found\r\n"
"Server: ESP8266\r\n"
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
    bool sentCommand = false;
    if (input.startsWith("+IPD,"))
       {
          Serial.println(input);
          int id =  input[5] - '0';
          String *answer;
          if (input.substring(input.indexOf('/'), input.lastIndexOf(' ')) == String("/")){ //read first line and check patch
            answer = &positiveAnswer;
            if (input.indexOf("POST") != -1)
              sentCommand = true;
          }
          else
            answer = &negativeAnswer;
          while (Serial1.available()>0){
            Serial.println(input = Serial1.readStringUntil('\n'));
            if (sentCommand && input.startsWith("command=")){ // parser
              Serial.println(input[input.indexOf('=')+1]);
              int pos = input.lastIndexOf('+');
              Serial.println(input.substring(input.indexOf('+') + 1, pos).toInt());
              Serial.println(input.substring(pos + 1, input.indexOf('&')).toInt());
            }
          }
          Serial1.println("AT+CIPSEND="+String(id)+","+answer->length());
          while (!Serial1.find(">"))
            delay(1);
          Serial.println(*answer);
          Serial1.print(*answer);
          while (!Serial1.find("OK"))
            delay(1);
          Serial1.println("AT+CIPCLOSE="+String(id));
          Serial1.find("OK");
       }
       else
    Serial.println(input);
  }
}
