
// Example of sleeping and saving power
// 
// Author: Nick Gammon
// Date:   25 May 2011

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <EEPROM.h>

#define LED_PIN 13
#define BUZZER 2
#define SOIL_ANALOG A0
#define SOIL_DIGITAL 3
#define POWER_OUT 4
#define LIGHT_PIN A1

#define VERSION "v0.2"
#define SLEEP_MINUTES 5
#define SOIL_RESISTANCE_THRESHOLD_DEFAULT_VALUE 200
#define SOIL_RESISTANCE_THRESHOLD_EEPROM_ADDRESS 0

int soil_resistance_threshold;

int addr = 0;
byte value;


// watchdog interrupt
ISR(WDT_vect) 
  {
  wdt_disable();  // disable watchdog
  }

void myWatchdogEnable(const byte interval) 
  {  
  MCUSR = 0;                          // reset various flags
  WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
  WDTCSR =  0b01000000 | interval;    // set WDIE, and appropriate delay

  wdt_reset();
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_mode();            // now goes to Sleep and waits for the interrupt
  } 

void setup()
{
  Serial.begin(115200);              //  setup serial
  Serial.println(VERSION);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(SOIL_ANALOG, INPUT);
  pinMode(SOIL_DIGITAL, INPUT);
  pinMode(POWER_OUT, OUTPUT);

  beep(600,500);
  beep(800,500);
  beep(1000,500);
  
  /*
  Serial.println("*** Initializing EERPOM with default SOIL_RESISTANCE_THRESHOLD value: " + String(SOIL_RESISTANCE_THRESHOLD_DEFAULT_VALUE));
  EEPROM.write(SOIL_RESISTANCE_THRESHOLD_EEPROM_ADDRESS, SOIL_RESISTANCE_THRESHOLD_DEFAULT_VALUE);
  value = EEPROM.read(SOIL_RESISTANCE_THRESHOLD_EEPROM_ADDRESS);
  Serial.println("Read SOIL_RESISTANCE_THRESHOLD_EEPROM: " + String(value));
  if (value == 0 || value == 255) {
    soil_resistance_threshold =  SOIL_RESISTANCE_THRESHOLD_DEFAULT_VALUE;
    EEPROM.write(SOIL_RESISTANCE_THRESHOLD_EEPROM_ADDRESS, soil_resistance_threshold);
  } else {
    soil_resistance_threshold = value;
  }
  */
  
  soil_resistance_threshold = SOIL_RESISTANCE_THRESHOLD_DEFAULT_VALUE;

}  // end of setup

void loop()
{
  digitalWrite (LED_PIN, HIGH);  // awake
  blinkLED_PIN();
  //delay (2000);    // ie. do stuff here
  digitalWrite (LED_PIN, LOW);  // asleep

  // turn on sensor's power line
  digitalWrite(POWER_OUT, HIGH);
  delay(3);
  // read the input pin
  int a = analogRead(SOIL_ANALOG);
  int b = digitalRead(SOIL_DIGITAL);
  int light_level = analogRead(LIGHT_PIN);
  // turn off sensor's power line
  digitalWrite(POWER_OUT, LOW);
  
  // debug value
  Serial.print("Light Level: ");
  Serial.println(light_level);
  
  Serial.print("Soil Resistance:");
  /*
  //Serial.print(" (DO=");
  Serial.print(b);
  Serial.print(") ");
  */
  Serial.print(" (");
  Serial.print(a);
  Serial.print(" < ");
  Serial.print(soil_resistance_threshold);
  Serial.print(") ");
  Serial.print(a < soil_resistance_threshold ? "OK" : "LOW!");
  Serial.println();

  // signal only in day-light
  // light level > 100 at evening indoor lamp on light
  // light level > 700 at daylight
  if (light_level > 100 && light_level < 755
      && (a >= soil_resistance_threshold || b == HIGH)) {
 /* Soui resistances:
   >160-225d when watered
   355 after > 1 week
 */
    beep(2000,20);
    delay(150);
    beep(2000,20);
  }

  Serial.flush();

  // sleep for a total of 20 seconds
  //myWatchdogEnable (0b100001);  // 8 seconds
  //myWatchdogEnable (0b100001);  // 8 seconds
  //myWatchdogEnable (0b100000);  // 4 seconds
  int k = SLEEP_MINUTES * 60 / 8;
  for (int i=0; i<k; i++) {
    myWatchdogEnable (0b100001);  // 8 seconds
  }

}  // end ofloop

void blinkLED_PIN() {
  digitalWrite(LED_PIN, HIGH);   // turn the LED_PIN on (HIGH is the voltage level)
  delay(2);                       // wait for a second
  digitalWrite(LED_PIN, LOW);    // turn the LED_PIN off by making the voltage LOW
  delay(100);                       // wait for a second
  digitalWrite(LED_PIN, HIGH);   // turn the LED_PIN on (HIGH is the voltage level)
  delay(2);                       // wait for a second
  digitalWrite(LED_PIN, LOW);    // turn the LED_PIN off by making the voltage LOW
  delay(2000);                       // wait for a second
}

void beep(int f, int t) {
  tone(BUZZER, f);
  delay(t);
  noTone(BUZZER);
}

// sleep bit patterns:
//  1 second:  0b000110
//  2 seconds: 0b000111
//  4 seconds: 0b100000
//  8 seconds: 0b100001
