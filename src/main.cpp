#include "SSD1306.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "WiFiClient.h"
#include "SPIFFS.h"

#include <Arduino.h>
#define LENG 31  //0x42 + 31 bytes equal to 32 bytes

#define RXD2 27
#define TXD2 26

unsigned char buf[LENG];

String PM01Value;  //define PM1.0 value of the air detector module
String PM2_5Value; //define PM2.5 value of the  air detector module
String PM10Value;

uint8_t ledPin = 16;
SSD1306 display(0x3c, 5, 4);


const char *ssid = "Ondrasz";
const char *password = "340@brij";

// Set your Static IP address
IPAddress local_IP(192, 168, 2,104);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //option



bool x = false;
String ipString;
String x1, x2_5, x10;
String y01;
String y2_5, y10;

AsyncWebServer server(80);    


String  transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val = ((thebuf[3] << 8) + thebuf[4]);
  return String(PM01Val);
}

//transmit PM Value to PC
String transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val = ((thebuf[5] << 8) + thebuf[6]);
  return String(PM2_5Val);
}

//transmit PM Value to PC
String transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val = ((thebuf[7] << 8) + thebuf[8]);
  return String(PM10Val + 1);
}

void setup()
{

  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);

  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

    // Initialize SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi

  WiFi.softAP(ssid, password);

  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(1000);
  //   Serial.println("Connecting to WiFi..");
  // }

  // Print ESP32 Local IP Address
   Serial.println(WiFi.softAPIP());

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
  // Route for root / web page
  server.begin();
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
 
}

void loop()
{
  Serial.println("Hello World");
  while (Serial1.available())
  {
    Serial.print(char(Serial1.read()), HEX);
  }
  Serial.println("Hello World");

  if (Serial1.find(0x42))
  { //start to read when detect 0x42
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
    // display.invertDisplay();

    display.setFont(ArialMT_Plain_16);
    display.drawString(10, 10, x1.c_str());
    display.drawString(70, 10, " = PM_01");

    Serial.println("  ug/m3");

    Serial.print("PM2.5: ");
    Serial.print(PM2_5Value);
    x2_5 = PM2_5Value;

    display.drawString(10, 30, x2_5.c_str());
    display.drawString(70, 30, " = PM_2.5");

    Serial.println("  ug/m3");

    Serial.print("PM1 0: ");
    Serial.print(PM10Value);
    x10 = PM10Value;
    display.drawString(10, 50, x10.c_str());
    display.drawString(70, 50, " = PM_10");
    Serial.println("  ug/m3");
    Serial.println();
    display.display();
    delay(10);
    display.clear();
  

  for (uint8_t n = 0; n < 5; n++)
  {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(100);
  }
}
