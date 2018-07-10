#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <stdlib.h> // For float to string conversion
#include <ESP8266HTTPClient.h>
#include "SDS011.h"
#include "RunningAverage.h"

unsigned long previousMillis = 0;
const long interval = 60000;
String p10str;
String p25str;
String url;
float p10,p25;
int error;

SDS011 my_sds;
RunningAverage myRApm25(20), myRApm10(20);

#define PIN_TX D2
#define PIN_RX D1
SoftwareSerial SerialSDS(PIN_RX, PIN_TX); // RX, TX

void setup() {
  Serial.begin(115200);
  Serial.println("Start Setup");
	my_sds.begin(PIN_TX,PIN_RX);

  WiFiManager wifiManager;
  wifiManager.autoConnect("SDS011WEMOS");

  myRApm10.clear();
  myRApm25.clear();
  
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
	error = my_sds.read(&p25,&p10);
	if (! error) {
		Serial.println("P2.5: "+String(p25));
		Serial.println("P10:  "+String(p10));
    myRApm25.addValue(p25);
    myRApm10.addValue(p10);
    Serial.println("P2.5 avg: "+String(myRApm25.getAverage()));
    Serial.println("P10 avg:  "+String(myRApm10.getAverage()));
	}
 
  unsigned long currentMillis = millis();
  // if enough millis have elapsed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // toggle the LED
    sendData();
  }
  
	delay(100);
}

void sendData() {
  //p10str = String(p10);
  //p25str = String(p25);
  p10str = String(myRApm10.getAverage());
  p25str = String(myRApm25.getAverage());
  HTTPClient http;
 
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  url = String("http://www.apeldoornindata.nl/data/senddata.php?id=AIDSENSORID&code=AIDSENSORCODE&location=AIDLOCATIONLAT,AIDLOCATIONLON&pm10=") + p10str + String("&pm25=") + p25str;
  Serial.print("[HTTP] ");
  Serial.println(url);
  http.begin(url); //HTTP
 
  
  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
 
    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
 
        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.print("[HTTP] ");
            Serial.println(payload);
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}


