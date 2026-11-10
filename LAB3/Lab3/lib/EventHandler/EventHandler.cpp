#include "EventHandler.hpp"

void EventHandler::Init(EventDef* eventDefs, uint16_t eventDefsCount) {

  this->eventDefsCount = eventDefsCount;
  for(uint16_t i = 0; i < eventDefsCount; i++) {
    this->eventDefs[i] = eventDefs[i];
  }

  eventQueue.Init();
}

bool EventHandler::Add(uint16_t eventNum) {

  if(eventNum >= eventDefsCount) {
    return false;
  }

  return eventQueue.Push(eventNum);
}

bool EventHandler::Handle() {

  uint16_t eventNum;
  if(!eventQueue.Pop(eventNum)){
    return false;
  }

  eventDefs[eventNum].eventFunc();
  return eventDefs[eventNum].important;
}
