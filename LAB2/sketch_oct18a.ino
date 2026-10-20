#include "LCD.h"
#include "EventHandler.h"
#include "SleepHandler.h"
#include <EEPROM.h>

EventHandler eventHandler;

const uint8_t DATA_PIN = A0;
bool dangerAlert = false;

enum class EVENTS : uint16_t {
  INCREASE_BUTTON_PRESS = 0,
  DECREASE_BUTTON_PRESS,
  SETTING_BUTTON_PRESS,
  SETTING_BUTTON_LONG_PRESS,
  PERIODIC_CHECK
};

const uint8_t MAGIC = 0xAF;
const int ADDR_MAGIC = 0;
const int ADDR_TEMP_CONFIG = 1;

const float LIGHT_LIMIT_DEFAULT = 200;
float lightLimit = 200;

void SaveSettings() {
  EEPROM.put(ADDR_TEMP_CONFIG, lightLimit);
  EEPROM.put(ADDR_MAGIC, MAGIC);
}

void LoadSettings() {
  if (EEPROM.read(ADDR_MAGIC) == MAGIC) {
    EEPROM.get(ADDR_TEMP_CONFIG, lightLimit);
  } else {
    lightLimit = LIGHT_LIMIT_DEFAULT;
    SaveSettings();
  }
}

bool settingModeActive = false;
volatile bool b4Held = false;

ISR(WDT_vect) {}

ISR(TIMER1_COMPA_vect) {
  if(settingModeActive) {
    return;
  }
  eventHandler.Add(static_cast<uint16_t>(EVENTS::PERIODIC_CHECK));
}

ISR(PCINT2_vect) {
  static uint8_t prev = 0xFF;
  static unsigned long last = 0, b4Start = 0;
  uint8_t now = PIND;
  unsigned long t = millis();
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

  Serial.print("INCREASE! ");
  Serial.println(lightLimit, 1);
}

void DecreaseButtonPress() {

  if(!settingModeActive) {
    return;
  }

  lightLimit -= 25;

  Serial.print("DECREASE! ");
  Serial.println(lightLimit, 1);
}

void SettingButtonPress() {

  if(!settingModeActive) {
    return;
  }

  Serial.print("Setting press.\n");
  settingModeActive = false;
  SaveSettings();
}

void SettingButtonLongPress() {

  if(settingModeActive) {
    return;
  }
  settingModeActive = true;
  Serial.print("Setting LONG press.\n");
  Serial.print("Current light config value: ");
  Serial.println(lightLimit, 1);
}

float ReadLight() {
  analogRead(DATA_PIN);
  int raw = analogRead(DATA_PIN);
  return raw;
}

void PeriodicCheck() {

  if(settingModeActive) {
    return;
  }

  float light = ReadLight();

  Serial.print("Light: ");
  Serial.println(light);

  if(light >= lightLimit) {
    Serial.println("TOO BRIGHT!");
    dangerAlert = true;
    digitalWrite(7, HIGH);
  }
  else {
    dangerAlert = false;
    digitalWrite(7, LOW);
  }
}

void SetupTimer1() {
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS12);
  OCR1A = 15625;
  TCNT1 = 0;
  TIMSK1 = (1 << OCIE1A);
}

void setup() {
  noInterrupts();

  // Clear config on restart if setting button pressed
  if (digitalRead(4) == LOW) { 
    EEPROM.update(ADDR_MAGIC, 0xFF);
  }

  pinMode(DATA_PIN, INPUT);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(7, OUTPUT);

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
    }
  };
  eventHandler.Init(eventDefs, 5);

  SleepHandler::Init();

  Serial.begin(9600);

  PCICR  |= (1<<PCIE2);
  PCMSK2 = 0b00011100;  // pins 2-7

  
  interrupts();
}

void loop() {

  static unsigned long lastActiveTime = 0;
  unsigned long now = millis();

  if(eventHandler.Handle() || b4Held || settingModeActive || dangerAlert) {
    lastActiveTime = now;
  }
  else if (now - lastActiveTime >= 2000) { // Inactive for 2 seconds
    SleepHandler::Sleep();
    lastActiveTime = now;
  }
}
