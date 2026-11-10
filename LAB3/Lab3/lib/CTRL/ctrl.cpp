#include "ctrl.hpp"

namespace CTRL {

  static volatile uint32_t timeMillis = 0;
  static uint16_t timeFraction = 0;

  // TODO: full clear pin drive
  static void disablePWM(Pin pin) {
    switch (pin) {
      case Pin::D5:  TCCR0A &= ~(1<<5); break; // COM0B1
      case Pin::D6:  TCCR0A &= ~(1<<7); break; // COM0A1
      case Pin::D9:  TCCR1A &= ~(1<<7); break; // COM1A1
      case Pin::D10: TCCR1A &= ~(1<<5); break; // COM1B1
      case Pin::D3:  TCCR2A &= ~(1<<5); break; // COM2B1
      case Pin::D11: TCCR2A &= ~(1<<7); break; // COM2A1
      default: break;
    }
  }

  PinRegs getPinRegs(Pin pin) {
    uint8_t pinNum = static_cast<uint8_t>(pin);
    if (pinNum < 8)  return { PIND, DDRD, PORTD, uint8_t(1 << pinNum) };
    if (pinNum < 14) return { PINB, DDRB, PORTB, uint8_t(1 << (pinNum - 8)) };
    return { PINC, DDRC, PORTC, uint8_t(1 << (pinNum - 14)) };
  }

  void pinMode(Pin pin, PinMode mode) {
    PinRegs pinRegs = getPinRegs(pin);
    switch (mode)
    {
      case PinMode::INPUT:
      {
        pinRegs.ddr  &= ~pinRegs.pinBit; 
        pinRegs.port &= ~pinRegs.pinBit;
        break;
      }

      case PinMode::OUTPUT:
      {
        pinRegs.ddr |= pinRegs.pinBit;
        break;
      }

      case PinMode::INPUT_PULLUP:
      {
        pinRegs.ddr  &= ~pinRegs.pinBit;
        pinRegs.port |= pinRegs.pinBit;
        break;
      }
    }                     
  }

  void digitalWrite(Pin pin, Level level) {
    disablePWM(pin);
    PinRegs pinRegs = getPinRegs(pin);
    switch (level)
    {
      case Level::HIGH:
      {
        pinRegs.port |= pinRegs.pinBit;
        break;
      }

      case Level::LOW:
      {
        pinRegs.port &= ~pinRegs.pinBit;
        break;
      }
    }
  }
  Level digitalRead(Pin pin) {
    PinRegs pinRegs = getPinRegs(pin);
    return (pinRegs.pin & pinRegs.pinBit) ? Level::HIGH : Level::LOW;
  }

  static bool isPWMPin(Pin pin) {
    switch (pin) {
      case Pin::D3: case Pin::D5: case Pin::D6:
      case Pin::D9: case Pin::D10: case Pin::D11:
        return true;
      default:
        return false;
    }
  }
  void analogWrite(Pin pin, uint8_t val) {
    if (!isPWMPin(pin)) return;
    if (val == 0)   { digitalWrite(pin, Level::LOW);  return; }
    if (val == 255) { digitalWrite(pin, Level::HIGH); return; }
    switch (pin) {
      case Pin::D5:  TCCR0A |= (1<<5)|(1<<1)|(1<<0); TCCR0B = (1<<1)|(1<<0); OCR0B  = val; break;
      case Pin::D6:  TCCR0A |= (1<<7)|(1<<1)|(1<<0); TCCR0B = (1<<1)|(1<<0); OCR0A  = val; break;
      case Pin::D9:  TCCR1A |= (1<<7)|(1<<0);        TCCR1B = (1<<1)|(1<<0); OCR1AL = val; break;
      case Pin::D10: TCCR1A |= (1<<5)|(1<<0);        TCCR1B = (1<<1)|(1<<0); OCR1BL = val; break;
      case Pin::D3:  TCCR2A |= (1<<5)|(1<<0);        TCCR2B = (1<<2);        OCR2B  = val; break;
      case Pin::D11: TCCR2A |= (1<<7)|(1<<0);        TCCR2B = (1<<2);        OCR2A  = val; break;
    }
  }
  int analogRead(Pin pin) {
    if(pin < Pin::A0) { return 0; }
    uint8_t ch = static_cast<uint8_t>(pin) - static_cast<uint8_t>(Pin::A0);

    // Select the A0 and A5 pins ad set voltage ref to 5V
    ADMUX  = (1<<6) | (ch & 0x07);

    // Enable ADC and set prescaler 128
    ADCSRA = (1<<7) | 0x07;

    // Start conversion
    ADCSRA |= (1<<6);

    // Wait for conversion to end
    while (ADCSRA & (1<<6));

    // Data is right aligned
    uint8_t lo = ADCL;
    return lo | (int(ADCH) << 8);
  }

  void serialBegin(uint32_t baud) {
    uint16_t ubrr = (F_CPU + baud * 8) / (baud * 16) - 1;
    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr & 0xFF;
    UCSR0A = 0;
    UCSR0B = (1<<3) | (1<<4);          // TXEN0, RXEN0
    UCSR0C = (1<<2) | (1<<1);          // 8 data bits, no parity, 1 stop
  }
  void serialWrite(char c) {
    while (!(UCSR0A & (1<<5)));   // UDRE0
    UCSR0A = (1<<6);              // clear TXC0 (U2X is 0)
    UDR0 = c;
  }
  void serialPrint(const char* str) {
    while (*str) serialWrite(*str++);
  }
  void serialPrint(int32_t num) {
    if (num == 0) { serialWrite('0'); return; }
    if (num < 0) { serialWrite('-'); num = -num; }
    char buf[10];
    uint8_t i = 0;
    while (num > 0) { buf[i++] = '0' + (num % 10); num /= 10; }
    while (i > 0) serialWrite(buf[--i]);
  }
  void serialPrintln(const char* str) { serialPrint(str); serialWrite('\r'); serialWrite('\n'); }
  void serialPrintln(int32_t num)     { serialPrint(num); serialWrite('\r'); serialWrite('\n'); }
  void serialPrint(float number, uint8_t digits) {
    if (number < 0.0f) { serialWrite('-'); number = -number; }

    float rounding = 0.5f;
    for (uint8_t i = 0; i < digits; i++) rounding /= 10.0f;
    number += rounding;

    uint32_t intPart = (uint32_t)number;
    float remainder = number - (float)intPart;
    serialPrint((int32_t)intPart);

    if (digits > 0) serialWrite('.');
    while (digits--) {
      remainder *= 10.0f;
      uint8_t digit = (uint8_t)remainder;
      serialWrite('0' + digit);
      remainder -= digit;
    }
  }
  void serialPrintln(float number, uint8_t digits) {
    serialPrint(number, digits);
    serialWrite('\r'); serialWrite('\n');
  }
  void serialFlush() {
    while (!(UCSR0A & (1<<6)));   // TXC0: shift register empty, all sent
  }

  uint8_t eepromRead(uint16_t addr) {
    while (EECR & (1<<1));            // wait EEPE (write finished)
    EEARH = addr >> 8;
    EEARL = addr & 0xFF;
    EECR |= (1<<0);                   // EERE: start read
    return EEDR;
  }

  void eepromWrite(uint16_t addr, uint8_t val) {
    while (EECR & (1<<1));            // wait EEPE
    EEARH = addr >> 8;
    EEARL = addr & 0xFF;
    EEDR  = val;

    // Save status register and disable interrupt because it can be interrupted after setting EEMPE
    uint8_t sreg = SREG;              // save SREG
    cli();
    EECR |= (1<<2);                   // EEMPE
    EECR |= (1<<1);                   // EEPE (within 4 cycles)
    SREG = sreg;                      // restore interrupts
  }

  void eepromUpdate(uint16_t addr, uint8_t val) {
    if (eepromRead(addr) != val) eepromWrite(addr, val);   // saves wear
  }

  void timerInit() {
    TCCR0A |= (1<<1) | (1<<0);   // fast PWM (same as analogWrite) does not need to be this
    TCCR0B  = (1<<1) | (1<<0);   // clk/64
    TIMSK0 |= (1<<0);            // TOIE0: overflow interrupt
    sei();
  }
  uint32_t millis() {
    uint8_t sreg = SREG;
    cli();                            // 4-byte read must be atomic
    uint32_t mil = timeMillis;
    SREG = sreg;
    return mil;
  }

  // Called by the ISR below
  void timer0Overflow() {
    uint32_t mil = timeMillis;
    uint16_t frac = timeFraction;
    mil += 1;
    frac += 24;
    if (frac >= 1000) { frac -= 1000; mil += 1; }
    timeMillis = mil;
    timeFraction = frac;
  }

}

// Source: TIMER0 OVF
ISR(__vector_16) {
  CTRL::timer0Overflow();
}