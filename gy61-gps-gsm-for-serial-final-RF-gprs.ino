#include <RH_ASK.h>
RH_ASK RF;
#include<SoftwareSerial.h>
SoftwareSerial Serial1(2, 3); //make RX arduino line is pin 2, make TX arduino line is pin 3.
SoftwareSerial gps(9, 10);
#include <TinyGPS.h>
TinyGPS GPS;
float lat, lon;
//#include <gprsData.h>
//gprsData myData;
String hostUrl = "intelidemics.com/gprs_send_dataGET";

#define x A1
#define y A2
#define z A3

int xsample = 0;
int ysample = 0;
int zsample = 0;

#define samples 10

#define minVal -50
#define MaxVal 50

#define trigPin A4
#define echoPin A5
#define buzzPin 12

char phone[] = "7387273070";

void setup()
{
  Serial1.begin(9600);
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzPin, OUTPUT);
  Serial.println("Railway Accident Alert  ");
  Serial.println("        System     ");
  delay(2000);
  Serial.print("Initializing,... Please Wait...");
  delay(1000);

  Serial.println("Initializing....");
  initGsmModule("AT", "OK", 1000);
  initGsmModule("ATE1", "OK", 1000);
  initGsmModule("AT+CPIN?", "READY", 1000);
  initGsmModule("AT+CMGF=1", "OK", 1000);
  initGsmModule("AT+CNMI=2,2,0,0,0", "OK", 1000);
  Serial.println("Initialized Successfully");
  delay(2000);
  Serial.println("Callibrating Accelerometer");
  for (int i = 0; i < samples; i++)
  {
    xsample += analogRead(x);
    ysample += analogRead(y);
    zsample += analogRead(z);
  }

  xsample /= samples;
  ysample /= samples;
  zsample /= samples;

  Serial.println(xsample);
  Serial.println(ysample);
  Serial.println(zsample);
  delay(1000);

  Serial.println("Waiting For GPS Signal");
  delay(2000);
  gps.begin(9600);
  gpsReceive();
  delay(2000);
  Serial.println("GPS is Ready");
  delay(1000);
  if (!RF.init())
  {
    Serial.println("RF Initialization Failed");
  }
  Serial.println("System Ready");
}

void loop()
{
  gpsReceive();
  rfReceive();
  delay(200);
  distance();

  int value1 = analogRead(x);
  int value2 = analogRead(y);
  int value3 = analogRead(z);

  int xValue = xsample - value1;
  int yValue = ysample - value2;
  int zValue = zsample - value3;

  Serial.print("x=");
  Serial.println(xValue);
  Serial.print("y=");
  Serial.println(yValue);
  Serial.print("z=");
  Serial.println(zValue);

  if (xValue < minVal || xValue > MaxVal  || yValue < minVal || yValue > MaxVal  || zValue < minVal || zValue > MaxVal)
  {
    Serial.println("Sending SMS");
    Send();
    Serial.println("SMS Sent");
    delay(2000);
    Serial.println("System Ready");
  }
 

}

void initGsmModule(String cmd, char *res, int t)
{
  while (1)
  {
    Serial.println(cmd);
    Serial1.println(cmd);
    delay(100);
    while (Serial1.available() > 0)
    {
      if (Serial1.find(res))
      {
        Serial.println(res);
        delay(t);
        return;
      }

      else
      {
        Serial.println("Error");
      }
    }
    delay(t);
  }
}


void Send()
{
  Serial1.println("AT");
  delay(500);
  serialPrint();
  Serial1.println("AT+CMGF=1");
  delay(500);
  serialPrint();
  Serial1.print("AT+CMGS=");
  Serial1.print('"');
  Serial1.print(phone);    //mobile no. for SMS alert
  Serial1.println('"');
  delay(500);
  serialPrint();
  Serial1.print("Latitude:");
  Serial1.println(lat);
  delay(500);
  serialPrint();
  Serial1.print(" longitude:");
  Serial1.println(lon);
  delay(500);
  serialPrint();
  delay(500);
  serialPrint();
  Serial1.print("http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=");
  Serial1.print(lat, 6);
  Serial1.print("+");              //28.612953, 77.231545   //28.612953,77.2293563
  Serial1.print(lon, 6);
  Serial1.write(26);
  delay(2000);
  serialPrint();
}

void serialPrint()
{
  while (Serial1.available() > 0)
  {
    Serial.print(Serial1.read());
  }
}


void gpsReceive()
{
  while (gps.available())    // While there is data on the RX pin...
  {
    int c = gps.read();    // load the data into a variable...
    if (GPS.encode(c))     // if there is a new valid sentence...
    {
      GPS.f_get_position(&lat, &lon);
      Serial.print(lat);
      Serial.print('\t');
      Serial.println(lon);
    }
  }
}

void distance()
{
  digitalWrite(trigPin, LOW);
  delay(2);
  digitalWrite(trigPin, HIGH);
  delay(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float dist = duration * (0.034 / 2);
  Serial.print("Distance: ");
  Serial.println(dist);
  delay(1000);
  if (dist < 45.0)
  {
    digitalWrite(buzzPin, HIGH);
    delay(1000);
  }
  else {
    digitalWrite(buzzPin, LOW);
  }
}


void rfReceive()
{
  byte buf[5];
  byte buflen = sizeof(buf);
  if (RF.recv(buf, &buflen))
  {
    Serial.print("Message is: ");
    Serial.println((char*)buf);
    if (buf[0] == 'R')Serial.println("Red Signal,,,Slows down the speed");
  }
}

