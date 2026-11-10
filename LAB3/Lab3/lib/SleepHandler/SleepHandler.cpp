#include "SleepHandler.hpp"
#include <ctrl.hpp>

using namespace CTRL;

void SleepHandler::Init() {
  cli();
  wdtReset();
  WDTCSR = (1<<4) | (1<<3);     // WDCE | WDE (unlock) have 4 cycles to complete set
  WDTCSR = (1<<6) | (1<<5);     // WDIE | WDP3 -> 4 s, interrupt mode
  sei();
}
void SleepHandler::Sleep() {
  wdtReset();
  SMCR = (1<<2);                // SM1 = power-down mode

  SMCR |= (1<<0);               // SE: sleep enable
  sleepCpu();
  SMCR &= ~(1<<0);              // SE off
}