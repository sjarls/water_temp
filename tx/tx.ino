// Include the required Arduino libraries:
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Define to which pin of the Arduino the 1-Wire bus is connected:
#define ONE_WIRE_BUS 2

// Create a new instance of the oneWire class to communicate with any OneWire device:
OneWire oneWire(ONE_WIRE_BUS);

// Pass the oneWire reference to DallasTemperature library:
DallasTemperature sensors(&oneWire);

RF24 radio(7, 8); // CE, CSN
const byte address[6] = "01488";
float payload[3];

unsigned long startTime;
unsigned long currentTime;
//const unsigned long quarterHour = 1500000;
const unsigned long quarterHour = 1000;

int n_samples = 96;
float samples[96];

float temp = 4.0;

float warmestHour = 2.0;
float warmestHourIndex = 0;

float warmestDay = 1.0;
float warmestDayIndex = 0;

int comLed = 4;


void setup() {
  // Begin serial communication at a baud rate of 9600:
  Serial.begin(9600);
  sensors.begin();

  pinMode(comLed, OUTPUT);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  startTime = millis();
}

void loop() {

  currentTime = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  
  // READ ONCE PER 25 MIN
  if (currentTime - startTime >= quarterHour) 
  {
    // READ TEMP
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0); // the index 0 refers to the first device

    // last element is newest reading and gets shifted one place towards first element every loop
    for (int i = 0; i < n_samples - 1; i++)
    {
      samples[i] = samples[i+1];
    }
    
    samples[n_samples-1] = temp;

    // GET WARMEST SAMPLE LAST HOUR
    for (int i = n_samples - 4; i < n_samples; i++)
    {
      if (warmestHour < samples[i] && samples[i] > 0 && samples[i] < 30)
      {
        warmestHour = samples[i];
        warmestHourIndex = i;
      }
    }

    // GET WARMEST SAMPLE LAST DAY: 
    for (int i = 0; i < n_samples; i++)
    {
      if (warmestDay < samples[i] && samples[i] > 0 && samples[i] < 30)
      {
        warmestDay = samples[i];
        warmestDayIndex = i;
      }
    }
    
    startTime = currentTime;
 
  }

  payload[0] = temp;
  payload[1] = warmestHour;
  payload[2] = warmestDay;
  
  bool comOK = radio.write(&payload, sizeof(payload));

  if (comOK)
  {
    digitalWrite(comLed, HIGH);
  }
  else 
  {
    digitalWrite(comLed, LOW);
  }
  

}
