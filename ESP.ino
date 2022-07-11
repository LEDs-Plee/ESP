/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/
#include "web.config.h"
#include "beneden.config.h"

#include <Arduino.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

#define PIN        4
#define NUMPIXELS 30
WiFiUDP ntpUDP;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500
NTPClient timeClient(ntpUDP);

const String protocol = "https://";
const String baseUrl = "wc.67.campuslaan.nl/status/";
const int id = WC_ID;
const String secret = WC_SECRET;
String url;
int freeState = 1;
int cArray[30];
int numVisits = 0;
int currentDay = 0;
const char* username = WEB_USER;
const char* password = WEB_PASS;
unsigned long startTime;
const int button = 5;
const int buttonGround = 0;
ESP8266WiFiMulti WiFiMulti;
uint32_t ColorMatrix[9] = {
  pixels.Color(0, 66, 4), // < 20 s / niet
  pixels.Color(0, 58, 12), // < 60 s
  pixels.Color(0, 40, 25), // < 90 s
  pixels.Color(0, 30, 30), // < 120 s
  pixels.Color(0, 0, 80), // < 200 s
  pixels.Color(40, 40, 0), // < 300 s
  pixels.Color(60, 30, 0), // < 600 s
  pixels.Color(50, 0, 40), // < 900 s
  pixels.Color(60, 60, 60) // > 900 s

};

void setup() {
  pinMode(button, INPUT_PULLUP);
  pinMode(buttonGround, OUTPUT);

  digitalWrite(buttonGround, LOW);

  pixels.begin();
  for (int i = 0; i < 9; i++) {
    pixels.fill(ColorMatrix[i], 0, NUMPIXELS);
    pixels.show();
    delay(1000);
  }
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  url = protocol + username + ":" + password + "@" + baseUrl + id + "/" + secret + "/";
  Serial.println(url);

  timeClient.begin();
}

void loop() {
  // wait for WiFi connection
  int reading = digitalRead(button);
  Serial.println(reading);

  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  Serial.println(timeClient.getDay());
  if (currentDay != timeClient.getDay()) { // Zondag -> Maandag : 0 -> 6 {
    currentDay = timeClient.getDay();
    numVisits = 0;
    for (int a = 0; a < 30; a++) {
      cArray[a] = 0;
    }
  }

  if (reading != freeState) {
    Serial.println("changing");

    if (reading == LOW) { // Door shutting
      startTime = millis();
      for (uint8_t i = 0; i <= 69; i++) {
        pixels.fill(pixels.Color(i, 70 - i, 5 * (1 - i / 70.0)), 0, NUMPIXELS);
        pixels.show();
        delay(12);
      }
      pixels.fill(pixels.Color(70, 0, 0), 0, NUMPIXELS);
    } else { // Door opening
      unsigned long sitTime = (millis() - startTime) / 1000;
      if (numVisits < 30) {
        cArray[numVisits] = (sitTime > 20) + (sitTime > 60) + (sitTime > 90) + (sitTime > 120) + (sitTime > 200) + (sitTime > 300) + (sitTime > 600) + (sitTime > 900);
        numVisits++;
        for (size_t i = 0; i < numVisits; i++) { // Shuffle
          size_t j = random(0, numVisits - i);
          int t = cArray[i];
          cArray[i] = cArray[j];
          cArray[j] = t;
        }
      }


      for (uint8_t i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 65, 5));
        pixels.show();
        delay(20);
      }
    }

    if ((WiFiMulti.run() == WL_CONNECTED)) {

      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

      client->setInsecure();

      HTTPClient https;
      https.begin(*client, url + freeState);
      https.GET();
      https.end();
    }
    freeState = reading;
  } else { // Steady State
    if (reading == HIGH) {// Door open (steady)
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, ColorMatrix[cArray[i]]);
      }
    }
    pixels.show();

  }

  delay(DELAYVAL);
}
