#include <ctrl.hpp>
#include <EventHandler.hpp>
#include <SleepHandler.hpp>
#include <lcd.hpp>
#include <stdio.h>
#include <buzzer.hpp>

EventHandler eventHandler;

const CTRL::Pin DATA_PIN = CTRL::Pin::A0;
bool dangerAlert = false;

enum class EVENTS : uint16_t {
  INCREASE_BUTTON_PRESS = 0,
  DECREASE_BUTTON_PRESS,
  SETTING_BUTTON_PRESS,
  SETTING_BUTTON_LONG_PRESS,
  PERIODIC_CHECK,
  UPDATE_SCREEN
};

const uint8_t MAGIC = 0xAF;
const int ADDR_MAGIC = 0;
const int ADDR_TEMP_CONFIG = 1;

const float LIGHT_LIMIT_DEFAULT = 200;
int lightLimit = 200;
int light = 0;

void SaveSettings() {
  CTRL::eepromPut(ADDR_TEMP_CONFIG, lightLimit);
  CTRL::eepromPut(ADDR_MAGIC, MAGIC);
}

void LoadSettings() {
  if (CTRL::eepromRead(ADDR_MAGIC) == MAGIC) {
    CTRL::eepromGet(ADDR_TEMP_CONFIG, lightLimit);
  } else {
    lightLimit = LIGHT_LIMIT_DEFAULT;
    SaveSettings();
  }
}

bool settingModeActive = false;
volatile bool b4Held = false;

// Watchdog interupt : WDT
ISR(__vector_6) {}
// This matches source : IMER1 COMPA
ISR(__vector_11) {
  if(settingModeActive) {
    return;
  }
  eventHandler.Add(static_cast<uint16_t>(EVENTS::PERIODIC_CHECK));
}
// Source: PCINT2
ISR(__vector_5) {

  static uint8_t prev = 0xFF;
  static unsigned long last = 0, b4Start = 0;
  uint8_t now = CTRL::PIND;
  unsigned long t = CTRL::millis();
  if (t - last < 30) return;
  uint8_t changed = prev ^ now;
  if (!changed) return;
  last = t;
  uint8_t fell = prev & ~now;
  uint8_t rose = ~prev & now;
  prev = now;

  if (fell & (1<<3)) {
    eventHandler.Add((uint16_t)EVENTS::INCREASE_BUTTON_PRESS);
  }
  if (fell & (1<<2)) {
    eventHandler.Add((uint16_t)EVENTS::DECREASE_BUTTON_PRESS);
  }
  
  if (fell & (1<<4)) {
    b4Held = true;
    b4Start = t;
  }
  if (rose & (1<<4)) {
    if (t - b4Start >= 1000)  {
      eventHandler.Add((uint16_t)EVENTS::SETTING_BUTTON_LONG_PRESS);
    }
    else {
      eventHandler.Add((uint16_t)EVENTS::SETTING_BUTTON_PRESS);
    } 
    b4Held = false;
  }
}

void IncreaseButtonPress() {

  if(!settingModeActive) {
    return;
  }

  lightLimit += 25;
  eventHandler.Add(static_cast<uint16_t>(EVENTS::UPDATE_SCREEN));
}

void DecreaseButtonPress() {

  if(!settingModeActive) {
    return;
  }

  lightLimit -= 25;
  eventHandler.Add(static_cast<uint16_t>(EVENTS::UPDATE_SCREEN));
}

void SettingButtonPress() {

  if(!settingModeActive) {
    return;
  }
  
  LCD::clearLine(LCD::Lines::Bottom);

  settingModeActive = false;
  SaveSettings();
}

void SettingButtonLongPress() {

  if(settingModeActive) {
    return;
  }
  settingModeActive = true;
  dangerAlert = false;

  eventHandler.Add(static_cast<uint16_t>(EVENTS::UPDATE_SCREEN));
}

void PeriodicCheck() {

  if(settingModeActive) {
    return;
  }

  light = analogRead(DATA_PIN);

  if(light >= lightLimit) {
    dangerAlert = true;
    CTRL::digitalWrite(CTRL::Pin::D7, CTRL::Level::HIGH);
  }
  else {
    dangerAlert = false;
    CTRL::digitalWrite(CTRL::Pin::D7, CTRL::Level::LOW);
  }

  eventHandler.Add(static_cast<uint16_t>(EVENTS::UPDATE_SCREEN));
}

void BuzzerUpdate() {
  static bool buzzOn = false;
  static unsigned long lastBuzz = 0;
  unsigned long now = CTRL::millis();
  if(dangerAlert) {
    if(now - lastBuzz >= 500) {
      if(buzzOn) {
        BUZZER::noTone();
      }
      else {
        BUZZER::tone(440);
      }
      lastBuzz = now;
      buzzOn = !buzzOn;
    }
    
  }
  else if (buzzOn) {
    BUZZER::noTone();
    buzzOn = false;
  }
}

void UpdateScreen() {

  char buffer[17];

  if(settingModeActive) {
    snprintf(buffer, sizeof(buffer), "Configuring...");
    LCD::writeLine(LCD::Lines::Top, buffer);

    snprintf(buffer, sizeof(buffer), "Max light:%4d", lightLimit);
    LCD::writeLine(LCD::Lines::Bottom, buffer);
  }
  else {
    snprintf(buffer, sizeof(buffer), "L:%4d  MAX:%4d", light, lightLimit);
    LCD::writeLine(LCD::Lines::Top, buffer);

    static unsigned long last = 0;
    static bool bottomOn = false;
    unsigned long now = CTRL::millis();

    if(dangerAlert) {
      // Need to flash the bottom line every second
      if(now - last >= 750) {
        // If the buttom row is on it will need to turn off
        if(bottomOn) {
          LCD::clearLine(LCD::Lines::Bottom);
        }
        // If off turn on
        else {
          LCD::writeLine(LCD::Lines::Bottom, "ALERT TOO BRIGHT");
        }
        bottomOn = !bottomOn;
        last = now;
      }
    }
    // If there is no danger and the bottom is on remove it
    else if (bottomOn) {
      LCD::clearLine(LCD::Lines::Bottom);
      bottomOn = false;
    }
  }
  
}

void SetupTimer1() {
  CTRL::TCCR1A = 0;
  CTRL::TCCR1B = (1 << 3) | (1 << 2);
  CTRL::OCR1A = 15625;
  CTRL::TCNT1 = 0;
  CTRL::TIMSK1 = (1 << 1);
}

void setup() {
  CTRL::cli();

  CTRL::serialBegin(9600);
  LCD::init();

  CTRL::pinMode(DATA_PIN, CTRL::PinMode::INPUT);
  CTRL::pinMode(CTRL::Pin::D2, CTRL::PinMode::INPUT_PULLUP);
  CTRL::pinMode(CTRL::Pin::D3, CTRL::PinMode::INPUT_PULLUP);
  CTRL::pinMode(CTRL::Pin::D4, CTRL::PinMode::INPUT_PULLUP);
  CTRL::pinMode(CTRL::Pin::D7, CTRL::PinMode::OUTPUT);

  // Clear config on restart if setting button pressed
  if (CTRL::digitalRead(CTRL::Pin::D4) == CTRL::Level::LOW) { 
    CTRL::eepromUpdate(ADDR_MAGIC, 0xFF);
  }

  SetupTimer1();

  LoadSettings();

  EventHandler::EventDef eventDefs[] = {
    EventHandler::EventDef {
      .eventFunc = IncreaseButtonPress,
      .important = true
    },
    EventHandler::EventDef {
      .eventFunc = DecreaseButtonPress,
      .important = true
    },
    EventHandler::EventDef {
      .eventFunc = SettingButtonPress,
      .important = true
    },
    EventHandler::EventDef {
      .eventFunc = SettingButtonLongPress,
      .important = true
    },
    EventHandler::EventDef {
      .eventFunc = PeriodicCheck,
      .important = false
    },
    EventHandler::EventDef {
      .eventFunc = UpdateScreen,
      .important = false
    }
  };
  eventHandler.Init(eventDefs, 6);

  SleepHandler::Init();

  CTRL::PCICR  |= (1<<2);
  CTRL::PCMSK2 = 0b00011100;  // pins 2-7

  CTRL::sei();
}

void loop() {

  static unsigned long lastActiveTime = 0;
  unsigned long now = CTRL::millis();

  // Can't have this in periodic update because 250ms jitters happen
  BuzzerUpdate();

  if(eventHandler.Handle() || b4Held || settingModeActive || dangerAlert) {
    lastActiveTime = now;
  }
  else if (now - lastActiveTime >= 6000) { // Inactive for 2 seconds
    // Before going to sleep and on wake up update LCD
    LCD::writeLine(LCD::Lines::Top, "Arduino Sleeping");
    SleepHandler::Sleep();
    lastActiveTime = now;
  }
}

int main() {
  CTRL::timerInit();
  setup();
  for(;;){
    loop();
  }
}
