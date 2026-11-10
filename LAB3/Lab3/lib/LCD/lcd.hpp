#pragma once
#include <stdint.h>

namespace LCD {
  enum class Lines : uint8_t { Top = 0x00, Bottom = 0x40 };

  void init(uint8_t address = 0x27);
  void writeLine(Lines line, const char* str);
  void clearLine(Lines line);
}