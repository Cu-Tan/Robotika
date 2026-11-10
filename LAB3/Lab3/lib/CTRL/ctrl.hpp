#pragma once
#include <stdint.h>

namespace CTRL {

  // PORT B GPIO (D8 - D13)
  static volatile uint8_t& PINB = *reinterpret_cast<volatile uint8_t*>(0x23);
  static volatile uint8_t& DDRB = *reinterpret_cast<volatile uint8_t*>(0x24);
  static volatile uint8_t& PORTB = *reinterpret_cast<volatile uint8_t*>(0x25);

  // PORT C GPIO (A0 - A5)
  static volatile uint8_t& PINC = *reinterpret_cast<volatile uint8_t*>(0x26);
  static volatile uint8_t& DDRC = *reinterpret_cast<volatile uint8_t*>(0x27);
  static volatile uint8_t& PORTC = *reinterpret_cast<volatile uint8_t*>(0x28);

  // PORT D GPIO (D0 - D7)
  static volatile uint8_t& PIND = *reinterpret_cast<volatile uint8_t*>(0x29);
  static volatile uint8_t& DDRD = *reinterpret_cast<volatile uint8_t*>(0x2A);
  static volatile uint8_t& PORTD = *reinterpret_cast<volatile uint8_t*>(0x2B);

  // Timer 0 regs
  static volatile uint8_t& TCCR0A = *reinterpret_cast<volatile uint8_t*>(0x44);
  static volatile uint8_t& TCCR0B = *reinterpret_cast<volatile uint8_t*>(0x45);
  static volatile uint8_t& OCR0A = *reinterpret_cast<volatile uint8_t*>(0x47);
  static volatile uint8_t& OCR0B = *reinterpret_cast<volatile uint8_t*>(0x48);

  static volatile uint8_t& TIMSK0 = *reinterpret_cast<volatile uint8_t*>(0x6E); // interupt mask

  // Timer 1 regs
  static volatile uint8_t& TCCR1A = *reinterpret_cast<volatile uint8_t*>(0x80);
  static volatile uint8_t& TCCR1B = *reinterpret_cast<volatile uint8_t*>(0x81);
  static volatile uint8_t& OCR1AL = *reinterpret_cast<volatile uint8_t*>(0x88);
  static volatile uint8_t& OCR1BL = *reinterpret_cast<volatile uint8_t*>(0x8A);

  static volatile uint8_t& TIMSK1 = *reinterpret_cast<volatile uint8_t*>(0x6F); // interupt mask

  // Timer 2 regs
  static volatile uint8_t& TCCR2A = *reinterpret_cast<volatile uint8_t*>(0xB0);
  static volatile uint8_t& TCCR2B = *reinterpret_cast<volatile uint8_t*>(0xB1);
  static volatile uint8_t& OCR2A = *reinterpret_cast<volatile uint8_t*>(0xB3);
  static volatile uint8_t& OCR2B = *reinterpret_cast<volatile uint8_t*>(0xB4);

  // For A0 - A5 analog reads
  static volatile uint8_t& ADCL = *reinterpret_cast<volatile uint8_t*>(0x78);
  static volatile uint8_t& ADCH = *reinterpret_cast<volatile uint8_t*>(0x79);
  static volatile uint8_t& ADCSRA = *reinterpret_cast<volatile uint8_t*>(0x7A);
  static volatile uint8_t& ADMUX = *reinterpret_cast<volatile uint8_t*>(0x7C);

  // For serial
  static volatile uint8_t& UCSR0A = *reinterpret_cast<volatile uint8_t*>(0xC0);
  static volatile uint8_t& UCSR0B = *reinterpret_cast<volatile uint8_t*>(0xC1);
  static volatile uint8_t& UCSR0C = *reinterpret_cast<volatile uint8_t*>(0xC2);
  static volatile uint8_t& UBRR0L = *reinterpret_cast<volatile uint8_t*>(0xC4);
  static volatile uint8_t& UBRR0H = *reinterpret_cast<volatile uint8_t*>(0xC5);
  static volatile uint8_t& UDR0 = *reinterpret_cast<volatile uint8_t*>(0xC6);

  // For sleeping the arduino
  static volatile uint8_t& WDTCSR = *reinterpret_cast<volatile uint8_t*>(0x60);
  static volatile uint8_t& SMCR = *reinterpret_cast<volatile uint8_t*>(0x53);

  // For EEPROM
  static volatile uint8_t& EEDR = *reinterpret_cast<volatile uint8_t*>(0x40);
  static volatile uint8_t& EEARL = *reinterpret_cast<volatile uint8_t*>(0x41);
  static volatile uint8_t& EEARH = *reinterpret_cast<volatile uint8_t*>(0x42);
  static volatile uint8_t& EECR = *reinterpret_cast<volatile uint8_t*>(0x3F);
  static volatile uint8_t& SREG = *reinterpret_cast<volatile uint8_t*>(0x5F);

  // TODO: group
  static volatile uint8_t& PCICR = *reinterpret_cast<volatile uint8_t*>(0x68);
  static volatile uint8_t& PCMSK2 = *reinterpret_cast<volatile uint8_t*>(0x6D);

  static volatile uint16_t& OCR1A = *reinterpret_cast<volatile uint16_t*>(0x88);
  static volatile uint16_t& OCR1B = *reinterpret_cast<volatile uint16_t*>(0x8A);
  static volatile uint16_t& TCNT1 = *reinterpret_cast<volatile uint16_t*>(0x84);

  // Some inline assembly
  inline void cli()      { __asm__ __volatile__("cli" ::: "memory"); }
  inline void sei()      { __asm__ __volatile__("sei" ::: "memory"); }
  inline void wdtReset() { __asm__ __volatile__("wdr"); }
  inline void sleepCpu() { __asm__ __volatile__("sleep" ::: "memory"); }

  // avr-gcc expects interupts in this form with name __vector_# (# being the 'interupt number - 1' from data sheet)
  #define ISR(vector) \
    extern "C" void vector(void) __attribute__((signal, used, externally_visible)); \
    void vector(void)

  enum class PinMode : uint8_t { INPUT, OUTPUT, INPUT_PULLUP };
  enum class Level : uint8_t { LOW, HIGH };

  enum class Pin : uint8_t {
    D0, D1, D2, D3, D4, D5, D6, D7,   // PORT D
    D8, D9, D10, D11, D12, D13,       // PORT B
    A0, A1, A2, A3, A4, A5            // PORT C
  };

  struct PinRegs {
    volatile uint8_t& pin;
    volatile uint8_t& ddr;
    volatile uint8_t& port;
    uint8_t pinBit;
  };

  PinRegs getPinRegs(Pin pin);

  void pinMode(Pin pin, PinMode mode);

  void digitalWrite(Pin pin, Level level);
  Level digitalRead(Pin pin);

  void analogWrite(Pin pin, uint8_t val);
  int analogRead(Pin pin);

  void serialBegin(uint32_t baud);
  void serialWrite(char c);
  void serialPrint(const char* str);
  void serialPrint(int32_t num);
  void serialPrintln(const char* str);
  void serialPrintln(int32_t num);
  void serialPrint(float number, uint8_t digits);
  void serialPrintln(float number, uint8_t digits);
  void serialFlush();

  uint8_t eepromRead(uint16_t addr);
  void    eepromWrite(uint16_t addr, uint8_t val);
  void    eepromUpdate(uint16_t addr, uint8_t val);

  template <typename T>
  void eepromPut(uint16_t addr, const T& value) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
    for (uint16_t i = 0; i < sizeof(T); i++) eepromUpdate(addr + i, p[i]);
  }

  template <typename T>
  void eepromGet(uint16_t addr, T& value) {
    uint8_t* p = reinterpret_cast<uint8_t*>(&value);
    for (uint16_t i = 0; i < sizeof(T); i++) p[i] = eepromRead(addr + i);
  }

  void timerInit();      // call once at start
  uint32_t millis();
}