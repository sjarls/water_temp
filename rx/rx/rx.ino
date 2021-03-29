#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// radio pins, addresse og initialisering av array for data
RF24 radio(7, 8); // CE, CSN
const byte address[6] = "01488";

float receiveData[3];

float tempNow;
float hourMax;
float dayMax;


void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    radio.read(&receiveData, sizeof(receiveData));
    
    tempNow = receiveData[0];
    hourMax = receiveData[1];
    dayMax = receiveData[2];
    
    Serial.print(receiveData[0]);
    Serial.print("\t");
    Serial.print(hourMax);
    Serial.print("\t");
    Serial.println(dayMax);
    
  }
}
