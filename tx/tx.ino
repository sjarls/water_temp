#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h> 
#define ONE_WIRE_BUS 2
#define LED_PIN (13)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RF24 radio(7, 8); // CE, CSN

const byte address[6] = "01488";
float payload[3];
unsigned long startTime;
unsigned long currentTime;
//const unsigned long periodTime = 900000;
const unsigned long periodTime = 1000;
int n_samples = 10;
float samples[10];
float temp = 0.0;
float coldestDay = 0.0;
float coldestDayIndex = 0;
float warmestDay = 0.0;
float warmestDayIndex = 0;

volatile int f_wdt=1;

ISR(WDT_vect)
{
  if(f_wdt == 0)
  {
    f_wdt=1;
  }
  else
  {
    Serial.println("WDT Overrun!!!");
  }
}

void enterSleep(void)
{
  digitalWrite(LED_PIN, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
  sleep_enable();
  
  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the WDT timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}

void setup() 
{
  Serial.begin(9600);
  Serial.println("Initialising...");
  delay(100); //Allow for serial print to complete.

  pinMode(LED_PIN,OUTPUT);
  sensors.begin();
  delay(200);
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(15,10);
  radio.openWritingPipe(address);
  radio.stopListening();

  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);

  Serial.println("Initialisation completed!");
  delay(100); //Allow for serial print to complete.
}

void loop() 
{
  if(f_wdt == 1) 
  {
    digitalWrite(LED_PIN, HIGH);
    getWaterTemp();
    payload[0] = temp;
    payload[1] = coldestDay;
    payload[2] = warmestDay;

    radio.write(&payload, sizeof(payload));
    Serial.println(payload[0]);

    delay(200);
    
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
  for (int i = 0; i < n_samples - 1; i++)
  {
    samples[i] = samples[i+1];
  }
  samples[n_samples-1] = temp;

  warmestDay = 0.0;
  coldestDay = 100.0;
  
  for (int i = 0; i < n_samples; i++)
  {
    if ((coldestDay > samples[i]) && (temp > 0))
    {
      coldestDay = samples[i];
      coldestDayIndex = i;
    }
  }
  
  for (int i = 0; i < n_samples; i++)
  {
    if ((warmestDay < samples[i]) && (temp > 0))
    {
      warmestDay = samples[i];
      warmestDayIndex = i;
    }
  }
  startTime = currentTime;
}
