#include "SleepHandler.h"
#include "Arduino.h"
#include <avr/sleep.h>
#include <avr/wdt.h>

void SleepHandler::Init() {
  cli();
  wdt_reset();
  WDTCSR = (1<<WDCE)|(1<<WDE);
  WDTCSR = (1<<WDIE)|(1<<WDP3);                 // 4 s
  sei();
}
void SleepHandler::Sleep() {
  wdt_reset();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  Serial.print("I sleep...\n");
  Serial.flush();

  sleep_enable(); 
  sleep_cpu(); 
  sleep_disable();

  Serial.print("I AWAKE!\n");
  Serial.flush();
}