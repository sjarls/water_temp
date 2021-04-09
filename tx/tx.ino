#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h> 
#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RF24 radio(7, 8);

const byte address[6] = "01488";
float payload[3];
float temp = 0.0;
volatile int f_wdt=1;

ISR(WDT_vect)
{
  if(f_wdt == 0)
  {
    f_wdt=1;
  }
}

void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sleep_enable();
  sleep_mode();
  sleep_disable();
  power_all_enable();
}

void setup() 
{
  sensors.begin();
  delay(200);
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(15,10);
  radio.openWritingPipe(address);
  radio.stopListening();

  MCUSR &= ~(1<<WDRF);
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = 1<<WDP0 | 1<<WDP3;
  WDTCSR |= _BV(WDIE);
}

void loop() 
{
  if(f_wdt == 1) 
  {
    getWaterTemp();
    payload[0] = temp;
    
    radio.write(&payload, sizeof(payload));
    
    f_wdt = 0;

    radio.powerDown();
    enterSleep();
    radio.powerUp();
  }
}

void getWaterTemp() 
{
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);
}
