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

#define PIN        4
#define NUMPIXELS 30

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500


const String protocol = "https://";
const String baseUrl = "wc.67.campuslaan.nl/status/";
const int id = 1;
const String secret = "SECRET";
String url;
int freeState = 0;

const char* username = "WEB_USER";
const char* password = "WEB_PASS";

const int button = 5;
//const int buttonGround = 4;
unsigned long startTime;
ESP8266WiFiMulti WiFiMulti;

void setup() {
  pinMode(button, INPUT_PULLUP);
//  pinMode(buttonGround, OUTPUT);

//  digitalWrite(buttonGround, LOW);

  pixels.begin();
  pixels.fill(pixels.Color(0,0,70), 0, NUMPIXELS);
  pixels.show();

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
  WiFiMulti.addAP("WIFI_SSID", "WIFI_PASS");
  url = protocol + username + ":" + password + "@" + baseUrl + id + "/" + secret + "/";
  Serial.println(url);
}

void loop() {
  // wait for WiFi connection
  int reading = digitalRead(button);
  Serial.println(reading);

  if (reading != freeState) {
    Serial.println("changing");

    if(reading == LOW) { // Door shutting
       startTime = millis();
       for (uint8_t i = 0; i <= 69; i++) {
         pixels.fill(pixels.Color(i,70-i,5*(1-i/70.0)), 0, NUMPIXELS);
         pixels.show();
         delay(12);  
       }
       pixels.fill(pixels.Color(65,5,0), 0, NUMPIXELS);
    } else { // Door opening
      for (uint8_t i = 0; i < NUMPIXELS; i++) {
         pixels.setPixelColor(i, pixels.Color(0,65,5));
         pixels.show();
         delay(20);
      }
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
  } else if (reading == LOW) { // Door shut (continuously updating)
     
      unsigned long sitTime = (millis() - startTime)/1000; // Time in seconds
     if (sitTime > 12) {
        if (sitTime < 650) {
           pixels.fill(pixels.Color(65-(sitTime/15),sitTime/10+5,0), 0, NUMPIXELS);
           pixels.show();
        } else {
           pixels.fill(pixels.Color(20,70,0), 0, NUMPIXELS);
           pixels.show();      
        }
     }
  }
     
  delay(DELAYVAL);
}
