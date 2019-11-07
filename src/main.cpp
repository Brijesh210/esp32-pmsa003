#include "SSD1306.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "WiFiClient.h"
#include "SPIFFS.h"

#include <Arduino.h>
#define LENG 31 //0x42 + 31 bytes equal to 32 bytes
#define RXD2 27 // PMS
#define TXD2 26 // PMS

unsigned char buf[LENG];

String PM01Value;  //define PM1.0 value of the air detector module
String PM2_5Value; //define PM2.5 value of the air detector module
String PM10Value;  //define PM10.0 value of the air detector module

uint8_t ledPin = 16;         // Blue Led
uint8_t setPin = 25;       // set pin
SSD1306 display(0x3c, 5, 4); // Display

const char *ssid = "Ondraszek";
const char *password = "340@brij";

IPAddress local_IP(192, 168, 2, 104); // Set your Static IP address
IPAddress gateway(192, 168, 1, 1);    // Set your Gateway IP address
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //option

bool x = true;
String x1, x2_5, x10;
String y01;
String y2_5, y10;
int int2_5 = 0; //  Global variable for Alert message

AsyncWebServer server(80);

String transmitPM01(unsigned char *thebuf) //Translating data for PM 1.0
{
  int PM01Val;
  PM01Val = ((thebuf[3] << 8) + thebuf[4]);
  return String(PM01Val);
}

String transmitPM2_5(unsigned char *thebuf) //Translating data for PM 2.5
{
  int PM2_5Val;
  PM2_5Val = ((thebuf[5] << 8) + thebuf[6]);
  return String(PM2_5Val);
}

String transmitPM10(unsigned char *thebuf) //Translating data for PM 10.0
{
  int PM10Val;
  PM10Val = ((thebuf[7] << 8) + thebuf[8]);
  return String(PM10Val + 1);
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  pinMode(ledPin, OUTPUT);
 
  pinMode(setPin, OUTPUT);
  digitalWrite(setPin, HIGH);
  //delay(10);
  //digitalWrite(setPin,LOW);
  

  WiFi.mode(WIFI_STA);
  //WiFi.softAP(ssid, password);
  WiFi.begin(ssid, password); //Wifi begin
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(2000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP()); // Display Local IP to serial port

  if (!SPIFFS.begin()) // Spiffs Initialize
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
  });
  server.on("/pm1", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", transmitPM01(buf).c_str());
  });
  server.on("/pm2.5", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", transmitPM2_5(buf).c_str());
  });
  server.on("/pm10", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", transmitPM10(buf).c_str());
  });
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.png", "image/png");
});

  server.begin(); // Server begin
  display.init(); //Display Initailize
  
 // display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
}

void loop()
{
  while(x == true)
  {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(58, 20, " Air Monitering");
    display.drawString(63, 50, WiFi.localIP().toString());
    display.display();
    delay(6000);
    display.clear();
    x = false;
  }
  

  // while (Serial1.available())
  // {
  //   Serial.print(char(Serial1.read()), HEX);
  // }

  if (Serial1.find(0x42)) //start to read when detect 0x42
  {
    Serial1.readBytes(buf, LENG);
    if (buf[0] == 0x4d)
    {
      PM01Value = transmitPM01(buf);   //count PM1.0 value of the air detector module
      PM2_5Value = transmitPM2_5(buf); //count PM2.5 value of the air detector module
      PM10Value = transmitPM10(buf);   //count PM10 value of the air detector module
    }
  }

  Serial.print("PM1.0: ");
  Serial.print(PM01Value);
  x1 = PM01Value;
  display.setFont(ArialMT_Plain_16);
  display.drawString(15, 10, x1.c_str());
  display.drawString(70, 10, " = PM_01");
  Serial.println("  ug/m3");

  Serial.print("PM2.5: ");
  Serial.print(PM2_5Value);
  x2_5 = PM2_5Value;
  display.drawString(15, 30, x2_5.c_str());
  display.drawString(70, 30, " = PM_2.5");

  Serial.println("  ug/m3");
  Serial.print("PM1 0: ");
  Serial.print(PM10Value);
  x10 = PM10Value;
  display.drawString(15, 50, x10.c_str());
  display.drawString(70, 50, " = PM_10");
  Serial.println("  ug/m3");
  Serial.println();

  display.display();
  delay(1000);
  display.clear();

  int2_5 = x2_5.toInt();
  if (int2_5 <= 50)
  {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(58, 35, "Best Air Quality");
    display.display();
    delay(10);
    display.clear();
  }
  else if (int2_5 > 50 || int2_5 <= 150)
  {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(58, 35, "Good Air Quality!");
    display.display();
    delay(10);
    display.clear();
  }
  else 
  {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(58, 35, "Poor Air Quality ");
    display.display();  
    delay(10);
    display.clear();
  }

  for (uint8_t n = 0; n < 5; n++)
  {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(100);
  }
  delay(3000);
}

 