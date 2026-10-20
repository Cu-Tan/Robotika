#pragma once

#include "Queue.h"

class EventHandler {

  public: struct EventDef {
    void (*eventFunc)();
    bool important; // if important is set to false then the arduino sleep timer is not reset
  };

  private: static const uint16_t MAX_EVENT_DEFS = 16;
  private: static const uint16_t MAX_EVENTS = 16;
  
  private: EventDef eventDefs[MAX_EVENT_DEFS];
  private: uint16_t eventDefsCount;

  private: Queue<uint16_t, MAX_EVENTS> eventQueue;

  public: void Init(EventDef* eventDefs, uint16_t eventDefsCount);
  public: bool Add(uint16_t eventNum);
  public: bool Handle();
};