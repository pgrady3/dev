#include "TimerOne.h"
#include "SPI.h"
#include "config.h"
#include "hall.h"

uint8_t useHalls = 0;

const uint8_t DO = 12;
const uint8_t CLK = 13;
const uint8_t CSnPin = 17;
const uint8_t A = 21;
const uint8_t B = 22;


void setup(){
  setupPins();
  pinMode(CSnPin,OUTPUT);
  pinMode(A, INPUT);
  pinMode(B, INPUT);


  digitalWrite(CSnPin, HIGH);
  digitalWrite(CSnPin, LOW);
  digitalWrite(CSnPin, HIGH);
  delay(100);
  
  if(useHalls)
    hallInit();

  hallSetThrottle(0.1);

  uint8_t curState = 0;
  while(1)
  {
    writeState(curState++);
    curState = curState % 6;
    

    for(uint16_t i = 0; i < 25; i++)
    {
      Serial.print(curState);
      Serial.print("\t");
      Serial.println(SPIread());
      delay(10);
    }
  }
}

void loop(){
  if(useHalls)
    hallPoll();
}

uint16_t SPIread()
{
  digitalWrite(CSnPin, LOW);
  delayMicroseconds(1);
  
  uint8_t d = SPI.transfer(0);
  uint16_t resp = d << 8;
  resp |= SPI.transfer(0);

  digitalWrite(CSnPin, HIGH);

  return resp >> 6;
}
