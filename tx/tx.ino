#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/sleep.h>
#include <avr/wdt.h> 
#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RF24 radio(7, 8); // CE, CSN

const byte address[6] = "01488";
float payload[3];
unsigned long startTime;
unsigned long currentTime;
//const unsigned long quarterHour = 900000;
const unsigned long quarterHour = 10000;
int n_samples = 96;
float samples[96];
float temp = 4.0;
float coldestDay = 50.0;
float coldestDayIndex = 0;
float warmestDay = 1.0;
float warmestDayIndex = 0;

void setup() 
{
  Serial.begin(9600);
 
  sensors.begin();
  
  radio.begin();
  radio.setRetries(15,10);
  radio.openWritingPipe(address);
  radio.stopListening();

  startTime = millis();
}

void loop() 
{
  getWaterTemp();
  
  payload[0] = temp;
  payload[1] = coldestDay;
  payload[2] = warmestDay;
  
  bool payloadSent = radio.write(&payload, sizeof(payload));
  
  delay(30);
  radio.powerDown();
  delayWDT(WDTO_8S);
  radio.powerUp(); 
}

void getWaterTemp() 
{
  currentTime = millis();
  if (currentTime - startTime >= quarterHour)
  {
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    for (int i = 0; i < n_samples - 1; i++)
    {
      samples[i] = samples[i+1];
    }
    samples[n_samples-1] = temp;
    for (int i = 0; i < n_samples; i++)
    {
      if ((coldestDay > samples[i]) && (samples[i] > 0) && (samples[i] < 30))
      {
        coldestDay = samples[i];
        coldestDayIndex = i;
      }
    }
    for (int i = 0; i < n_samples; i++)
    {
      if ((warmestDay < samples[i]) && (samples[i] > 0) && (samples[i] < 30))
      {
        warmestDay = samples[i];
        warmestDayIndex = i;
      }
    }
    startTime = currentTime;
  }
}

void delayWDT(byte timer) 
{
  sleep_enable(); 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  ADCSRA &= ~(1<<ADEN); 
  WDTCSR |= 0b00011000;
  WDTCSR =  0b01000000 | timer;
  wdt_reset();
  sleep_cpu();
  sleep_disable();
  ADCSRA |= (1<<ADEN);
}

ISR (WDT_vect) 
{
  wdt_disable();
  MCUSR = 0;
}
