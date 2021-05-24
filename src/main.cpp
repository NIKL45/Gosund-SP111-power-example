#include <Arduino.h>
//#include "EasyOta.h"
#include <ESP8266WiFi.h>
#include <credentials.h>
#include <TelnetStream.h>

IPAddress ip(192, 168, 2, 19);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);

#define WIFI_SSID mySSID_2
#define WIFI_PASSWORD myPASSWORD_2

bool SerDebug = 1;

#define buttonPin 13
#define relayPin 15
#define LEDredPin 0
#define LEDbluePin 2

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void handleTelnetCMDs()
{
  switch (TelnetStream.read())
  {
  case 'R':
    TelnetStream.stop();
    delay(100);
    ESP.reset();
    break;
  case 'C':
    TelnetStream.println("bye bye");
    TelnetStream.flush();
    //TelnetStream.stop();
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{

  if (SerDebug)
  {
    Serial.begin(9600);
  }
  Serial.println();
  Serial.println("Booting... ");

  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("WiFi failed, retrying.");
  }

  //EasyOta.setup();
  TelnetStream.begin();
  TelnetStream.println("Booted");
  Serial.println("Booted");

  ////////////////////////
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(LEDredPin, OUTPUT);
  pinMode(LEDbluePin, OUTPUT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  //EasyOta.checkForUpload();
  handleTelnetCMDs();
  //TelnetStream.println(millis());
  //delay(100);

  if (digitalRead(buttonPin) == LOW)
  {
    digitalWrite(relayPin, HIGH);
  }
  else
  {
    digitalWrite(relayPin, LOW);
  }
}
