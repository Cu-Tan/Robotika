// lcd.cpp
#include "lcd.hpp"
#include <util/delay.h>

/*
PCF8574 byte layout B backlight
7  6  5  4  3  2  1  0
D7 D6 D5 D4 BL E  RW RS
*/

namespace LCD {

  static volatile uint8_t& TWBR = *reinterpret_cast<volatile uint8_t*>(0xB8);
  static volatile uint8_t& TWSR = *reinterpret_cast<volatile uint8_t*>(0xB9);
  static volatile uint8_t& TWDR = *reinterpret_cast<volatile uint8_t*>(0xBB);
  static volatile uint8_t& TWCR = *reinterpret_cast<volatile uint8_t*>(0xBC);

  static uint8_t lcdAddr = 0x27;
  static constexpr uint8_t WIDTH = 16;

  static void i2cStart() {
    TWCR = (1<<7) | (1<<5) | (1<<2);
    while (!(TWCR & (1<<7)));
  }
  static void i2cStop() {
    TWCR = (1<<7) | (1<<4) | (1<<2);
  }
  static void i2cWrite(uint8_t b) {
    TWDR = b;
    TWCR = (1<<7) | (1<<2);
    while (!(TWCR & (1<<7)));
  }

  // Bit of an oversight there is no ACK bit check here...
  static void sendNibble(uint8_t nib, uint8_t rs) {
    uint8_t d = (nib << 4) | (1 << 3) | rs;
    i2cStart();
    i2cWrite(lcdAddr << 1);
    i2cWrite(d | (1 << 2));  // E high
    i2cWrite(d);             // E low: latch
    i2cStop();
    _delay_us(50);
  }
  static void sendByte(uint8_t v, uint8_t rs) {
    sendNibble(v >> 4, rs);
    sendNibble(v & 0b00001111, rs);
  }

  void init(uint8_t address) {
    lcdAddr = address;
    TWSR = 0;
    TWBR = 72;

    _delay_ms(50);
    // Send 3 function set command to 8 bit mode if the MCU resets mid send on 4 bit mode
    sendNibble(0b0011, 0); _delay_ms(5);
    sendNibble(0b0011, 0); _delay_us(150);
    sendNibble(0b0011, 0); _delay_us(150);
    // send 4 bit mode nibble
    sendNibble(0b0010, 0);

    // Send 4 bit mode again with line bit and style
    sendByte(0b00101000, 0);

    // Display switch (display on, cursor off)
    sendByte(0b00001100, 0);

    // Clear screen
    sendByte(0b00000001, 0); _delay_ms(2);

    // Shift cursor to right on entry
    sendByte(0b00000110, 0);
  }

  void writeLine(Lines line, const char* str) {
    sendByte((1 << 7) | static_cast<uint8_t>(line), 0);   // set DDRAM address
    for (uint8_t i = 0; i < WIDTH; i++) {
      sendByte(*str ? *str++ : ' ', 1);
    }
  }

  void clearLine(Lines line) {
    writeLine(line, "");
  }
}