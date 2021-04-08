/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com  
*********/

// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>S
#endif

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Radio
RF24 radio(D4, D8); // CE, CSN
const byte address[6] = "01488";

float receiveData[3];

// Variables to store temperature values
String tempNow = "";
String tempMax24 = "";
String tempMin24 = "";

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 30000;

// Replace with your network credentials
const char* ssid = "Telenor7889nye";
const char* password = "nqidqnwlbpxbn";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: middle;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Kastholmen Vanntemp</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Current</span>
    <span id="tempnow">%TEMPNOW%</span> 
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#ff0000;"></i> 
    <span class="ds-labels">Max 24h</span>
    <span id="tempmax24">%TEMPMAX24%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#0000ff;"></i> 
    <span class="ds-labels">Min 24h</span>
    <span id="tempmin24">%TEMPMIN24%</span> 
    <sup class="units">&deg;C</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tempnow").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/tempnow", true);
  xhttp.send();
}, 10000) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tempmax24").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/tempmax24", true);
  xhttp.send();
}, 10000) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DS18B20 values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPNOW"){
    return tempNow;
  }
  else if(var == "TEMPMAX24"){
    return tempMax24;
  }
  else if(var == "TEMPMIN24"){
    return tempMin24;
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/tempnow", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", tempNow.c_str());
  });
  server.on("/tempmax24", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", tempMax24.c_str());
  });
  server.on("/tempmin24", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", tempMin24.c_str());
  });
  // Start server
  server.begin();

    //Radio Setup
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

}
 
void loop(){
  if (radio.available()) {
    radio.read(&receiveData, sizeof(receiveData));
    
    tempNow = receiveData[0];
    tempMin24 = receiveData[1];
    tempMax24 = receiveData[2];

    Serial.println(tempNow);
    
  }

}
