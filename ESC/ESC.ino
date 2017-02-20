#include "TimerOne.h"
#include "SPI.h"
#include "config.h"
#include "hall.h"

uint8_t useHalls = 0;

void setup(){
  setupPins();
  
  if(useHalls)
    hallInit();

  hallSetThrottle(0.1);

  uint8_t curState = 0;
  while(1)
  {
    writeState(curState);
    curState = curState % 6;
    delay(1000);
  }
}

void loop(){
  if(useHalls)
    hallPoll();
}

