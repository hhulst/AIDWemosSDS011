// PM10/2.5 Dust sensor + Temp/Hum Sensor.

// Used components:
// - WEMOS D1 mini Pro, 80MHz, 16MB.
// - SDS011 Dust sensor
// - SparkFun HTU21D Temp/Hum sensor

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <stdlib.h>               // For float to string conversion
#include <ESP8266HTTPClient.h>
#include "SDS011.h"
#include "RunningAverage.h"
#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

float temperature, humidity, pressure, altitude;


// BME280 I2C address is 0x76(108)
#define Addr 0x76

// double cTemp = 0, fTemp = 0, pressure = 0, humidity = 0;

unsigned long previousMillis = 0;
const long interval = 60000;
String p10str;
String p25str;
String pstr;
String IDXpm;
String pstrPM;
String url;
String urlpm;
String pstrTemp, pstrHum, pstrPres;
String humS, tempS;
float p10,p25;
int error;
String DomoticzServer = "<IP address>";
String DSPort = "<Port>";
String PM10IDX = "<IDX1>";
String PM25IDX = "<IDX2>";
String TempHumIDX = "<IDX3>";
String AIDid = "<ID>";
String AIDcode = "<CODE>";
String AIDlat = "<LAT>";
String AIDlon = "<LON>";

SDS011 my_sds;
RunningAverage myRApm25(20), myRApm10(20);

#define PIN_TX D6
#define PIN_RX D5
SoftwareSerial SerialSDS(PIN_RX, PIN_TX); // RX, TX

//Create an instance of the object
HTU21D myHumidity;


void setup() {
  myHumidity.begin();
  Serial.begin(115200);
  Serial.println("Start Setup");
  my_sds.begin(PIN_TX,PIN_RX);
  WiFiManager wifiManager;
  wifiManager.autoConnect("SDS011WEMOS");
  myRApm10.clear();
  myRApm25.clear();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
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

  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();
  tempS = String(temp);
  humS = String(humd);

  Serial.print(" Temperature:");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print(" Humidity:");
  Serial.print(humd, 1);
  Serial.print("%");
  Serial.println();
 
  unsigned long currentMillis = millis();
  // if enough millis have elapsed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    delay(100);
    // toggle the LED
    sendAIDData();
    pstrPM = String(myRApm10.getAverage());
    sendPMData(pstrPM,PM10IDX);
    pstrPM = String(myRApm25.getAverage());
    sendPMData(pstrPM,PM25IDX);  
    pstrTemp = String(temp);
    pstrHum = String(humd);
    sendTempHumData(pstrTemp, pstrHum, TempHumIDX);  
  }
  delay(1000);
}

void sendAIDData() {
  p10str = String(myRApm10.getAverage());
  p25str = String(myRApm25.getAverage());
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  url = String("http://www.apeldoornindata.nl/data/senddata.php?id=") + AIDid + String("&code=") + AIDcode + String("&location=") + AIDlat + String(",") + AIDlon + String("&pm10=") + p10str + String("&pm25=") + p25str + String("&temp=") + tempS + String("&rh=") + humS;
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


void sendPMData(String myRApm, String IDX) {
  pstr = myRApm;
  IDXpm = IDX;
  p10str = String(myRApm10.getAverage());
  p25str = String(myRApm25.getAverage());
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  urlpm = String("http://") + DomoticzServer + String(":") + DSPort + String("/json.htm?type=command&param=udevice&idx=") +IDXpm + String("&nvalue=0") + String("&svalue=") + pstr;
  Serial.print("[HTTP] ");
  Serial.println(urlpm);
  http.begin(urlpm); //HTTP
  Serial.print("[HTTP] GET...\n");
  int httpCode = http.GET();
  // httpCode will be negative on error
  if(httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
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


void sendTempHumData(String myTemp, String myHum, String IDX) {
  IDXpm = IDX;
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  urlpm = String("http://") + DomoticzServer + String(":") + DSPort + String("/json.htm?type=command&param=udevice&idx=") +IDXpm + String("&nvalue=0") + String("&svalue=") + myTemp + String(";") + myHum + String(";0");
  Serial.print("[HTTP] ");
  Serial.println(urlpm);
  http.begin(urlpm); //HTTP
  Serial.print("[HTTP] GET...\n");
  int httpCode = http.GET();
  if(httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
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
