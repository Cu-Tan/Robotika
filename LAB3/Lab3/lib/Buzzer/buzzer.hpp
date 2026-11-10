// buzzer.hpp
#pragma once
#include <stdint.h>

namespace BUZZER {
  void tone(uint16_t freq);   // Hz, 0 = stop
  void noTone();
}