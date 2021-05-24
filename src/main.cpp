#include <Arduino.h>
//#include "EasyOta.h"
#include <ESP8266WiFi.h>
#include <credentials.h>
#include <TelnetStream.h>
#include "HLW8012.h"

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

#define SEL_PIN 12
#define CF1_PIN 4
#define CF_PIN 5

// Set SEL_PIN to LOW to sample current on the HJL-01
#define CURRENT_MODE LOW
#define UPDATE_TIME 3000

// These are experimental ratios for the BLITZWOLF BWSHP2 using the HJL-01
#define HJL01_CURRENT_RATIO 23142.2
#define HJL01_VOLTAGE_RATIO 263409.3
#define HJL01_POWER_RATIO 2846663.8

HLW8012 hjl01;

// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void ICACHE_RAM_ATTR hjl01_cf1_interrupt()
{
  hjl01.cf1_interrupt();
}
void ICACHE_RAM_ATTR hjl01_cf_interrupt()
{
  hjl01.cf_interrupt();
}

void calibrate()
{

  // Let's first read power, current and voltage
  // with an interval in between to allow the signal to stabilise:

  hjl01.getActivePower();

  hjl01.setMode(MODE_CURRENT);
  unsigned long timeout = millis();
  while ((millis() - timeout) < 2000)
  {
    delay(1);
  }
  hjl01.getCurrent();
  //////////////////
  hjl01.setMode(MODE_VOLTAGE);
  while ((millis() - timeout) < 2000)
  {
    delay(1);
  }
  hjl01.getVoltage();

  // Calibrate using a 60W bulb (pure resistive) on a 230V line
  hjl01.expectedActivePower(55.7);
  hjl01.expectedVoltage(232);
  hjl01.expectedCurrent(55.7 / 232);

  // Show corrected factors
  TelnetStream.println("[HLW] New current multiplier : " + String(hjl01.getCurrentMultiplier()));
  TelnetStream.println("[HLW] New voltage multiplier : " + String(hjl01.getVoltageMultiplier()));
  TelnetStream.println("[HLW] New power multiplier   : " + String(hjl01.getPowerMultiplier()));

  // Show corrected factors
  Serial.println("[HLW] New current multiplier : " + String(hjl01.getCurrentMultiplier()));
  Serial.println("[HLW] New voltage multiplier : " + String(hjl01.getVoltageMultiplier()));
  Serial.println("[HLW] New power multiplier   : " + String(hjl01.getPowerMultiplier()));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void handleTelnetCMDs()
{
  switch (TelnetStream.read())
  {
  case 'r':
    TelnetStream.stop();
    delay(100);
    ESP.reset();
    break;
  case 'c':
    TelnetStream.println("bye bye");
    TelnetStream.flush();
    TelnetStream.stop();
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

  Serial.println("\nBooting... ");

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
  TelnetStream.println("Booted\n\n");
  Serial.println("Booted\n\n");

  ////////////////////////
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(LEDredPin, OUTPUT);
  pinMode(LEDbluePin, OUTPUT);
  digitalWrite(LEDbluePin, HIGH);

  digitalWrite(relayPin, HIGH);

  ////////////////////////

  // Initialize HLW8012
  // void begin(unsigned char cf_pin, unsigned char cf1_pin, unsigned char sel_pin, unsigned char currentWhen = HIGH, bool use_interrupts = false, unsigned long pulse_timeout = PULSE_TIMEOUT);
  // * cf_pin, cf1_pin and sel_pin are GPIOs to the HLW8012 IC
  // * currentWhen is the value in sel_pin to select current sampling
  // * set use_interrupts to true to use interrupts to monitor pulse widths
  // * leave pulse_timeout to the default value, recommended when using interrupts
  hjl01.begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, true);

  // Set the ratios
  hjl01.setCurrentMultiplier(HJL01_CURRENT_RATIO);
  hjl01.setVoltageMultiplier(HJL01_VOLTAGE_RATIO);
  hjl01.setPowerMultiplier(HJL01_POWER_RATIO);

  // Set the interrupts on FALLING
  attachInterrupt(digitalPinToInterrupt(CF1_PIN), hjl01_cf1_interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(CF_PIN), hjl01_cf_interrupt, FALLING);

  // Show factors
  Serial.println("[HLW] current multiplier : " + String(hjl01.getCurrentMultiplier()));
  Serial.println("[HLW] voltage multiplier : " + String(hjl01.getVoltageMultiplier()));
  Serial.println("[HLW] power multiplier   : " + String(hjl01.getPowerMultiplier()));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  //EasyOta.checkForUpload();
  handleTelnetCMDs();

  static unsigned long last = millis();

  // This UPDATE_TIME should be at least twice the minimum time for the current or voltage
  // signals to stabilize. Experimentally that's about 1 second.
  if ((millis() - last) > UPDATE_TIME)
  {

    last = millis();
    TelnetStream.print("[HLW] Active Power (W)    : ");
    TelnetStream.println(hjl01.getActivePower());
    TelnetStream.print("[HLW] Voltage (V)         : ");
    TelnetStream.println(hjl01.getVoltage());
    TelnetStream.print("[HLW] Current (A)         : ");
    TelnetStream.println(hjl01.getCurrent());
    TelnetStream.print("[HLW] Apparent Power (VA) : ");
    TelnetStream.println(hjl01.getApparentPower());
    TelnetStream.print("[HLW] Power Factor (%)    : ");
    TelnetStream.println((int)(100 * hjl01.getPowerFactor()));
    TelnetStream.print("[HLW] Agg. energy (Ws)    : ");
    TelnetStream.println(hjl01.getEnergy());
    TelnetStream.println();
  }

  if (digitalRead(buttonPin) == LOW)
  {
    //calibrate();
  }
}
