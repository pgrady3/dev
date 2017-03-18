#include "TimerOne.h"
#include "SPI.h"
#include "config.h"
#include "hall.h"
#include "enc.h"

void setup(){
  init();
  ENCinit();

  if(0)//MAGNETIC VARIATION TEST
    ENCvarTest();

  if(1)
    ENClinTest();
}

void loop(){
  
  uint16_t data = ENCread();
  uint16_t pos = data >> 6;
  uint8_t flags = data & 0x3F;
  Serial.println(pos);
  Serial.println(flags>>1, BIN);
  
  digitalWrite(LED2, !digitalRead(LED2));
  delay(100);
}



