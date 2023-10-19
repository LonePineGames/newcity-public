#pragma once

enum Event {
  #define EVENT(N) Event##N,
  #include "eventsEnum.hpp"
  #undef EVENT
  numEvents
};

void fireEvent(Event event);
void initEvents();

