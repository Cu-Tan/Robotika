// buzzer.cpp
#include "buzzer.hpp"
#include <ctrl.hpp>

namespace BUZZER {

  void tone(uint16_t freq) {
    if (freq == 0) { noTone(); return; }

    static const uint16_t div[] = {1, 8, 32, 64, 128, 256, 1024};
    static const uint8_t  cs[]  = {1, 2, 3, 4, 5, 6, 7};

    uint8_t i = 0;
    uint32_t top = 0;
    for (; i < 7; i++) {
      top = F_CPU / (2UL * div[i] * freq);
      if (top >= 1 && top <= 256) break;
    }
    if (i == 7) { noTone(); return; }

    // Set pin 11 as output
    CTRL::DDRB  |= (1 << 3);
    // Toggle OC2A on compare match
    // Toggle CTC mode
    CTRL::TCCR2A = (1 << 6) | (1 << 1);
    CTRL::OCR2A  = top - 1;
    CTRL::TCCR2B = cs[i];                   // prescaler
  }

  void noTone() {
    CTRL::TCCR2B = 0;
    CTRL::TCCR2A = 0;
    // Turn off pin 11
    CTRL::PORTB &= ~(1 << 3);
  }
}