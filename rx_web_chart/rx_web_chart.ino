/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WiFiClientSecure.h>

RF24 radio(D4, D8); // CE, CSN
const byte address[6] = "01488";
float receiveData[3];
float t;
float h;

unsigned long currentMillis, prevMillis;
int reportInterval = 10000;

const char* ssid = "Telenor7889nye";
const char* password = "nqidqnwlbpxbn";

WiFiClientSecure client;
const char* host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "AKfycbwJ530ci2wpcYHoLCxlKOFO9FiRKwYYXVeYuZ9K952PBo9fCHpU";

AsyncWebServer server(80);

String readTemp() 
{
  return String(t);
}

void setup(){
  Serial.begin(115200);
    
  bool status; 

  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    delay(1000);
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemp().c_str());
  });

  server.begin();

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  client.setInsecure();
}
 
void loop()
{
 bool dataAvailable = radio.available();
 if (dataAvailable) 
 {
    radio.read(&receiveData, sizeof(receiveData));
    t = receiveData[0];
    h = 100.0;
    
    if ((currentMillis - prevMillis) > reportInterval) 
    {
      reportGoogleDocs();
      prevMillis = currentMillis;
    }
 }

 Serial.println(dataAvailable);

 currentMillis = millis();
 
 delay(20);
}

void reportGoogleDocs() {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  
  String string_temperature =  String(t);
  String string_humidity =  String(h); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
} 
