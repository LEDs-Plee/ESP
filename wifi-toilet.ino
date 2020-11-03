/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>

#include <Adafruit_NeoPixel.h>

#define PIN        2
#define NUMPIXELS 30

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500


const String protocol = "https://";
const String baseUrl = "wc.67.campuslaan.nl/status/";
const int id = 2;
const String secret = "fvlFQ76AQX";
String url;
int freeState = 0;

const char* username = "boven";
const char* password = "toiletboven";

const int button = 5;
const int buttonGround = 4;

ESP8266WiFiMulti WiFiMulti;

void setup() {
  pinMode(button, INPUT_PULLUP);
  pinMode(buttonGround, OUTPUT);

  digitalWrite(buttonGround, LOW);

  pixels.begin();

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
  WiFiMulti.addAP("The Amazing SpiderLAN", "StarkPassword3!");
  url = protocol + username + ":" + password + "@" + baseUrl + id + "/" + secret + "/";
  Serial.println(url);
}

void loop() {
  // wait for WiFi connection
  int reading = digitalRead(button);
  Serial.println(reading);

  if (reading != freeState) {

    if(reading == LOW) {
      pixels.fill(pixels.Color(70,0,0), 0, NUMPIXELS);
    } else {
      pixels.fill(pixels.Color(0,70,0), 0, NUMPIXELS);
    }
    pixels.show();

    freeState = reading;
    
    if ((WiFiMulti.run() == WL_CONNECTED)) {

      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

      client->setInsecure();

      HTTPClient https;
      https.begin(*client, url + freeState);
      https.GET();
      https.end();
    }
  }
  delay(DELAYVAL);
}
