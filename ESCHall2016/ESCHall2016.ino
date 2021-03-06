//uint8_t hallOrder[] = {255, 0, 4, 5, 2, 1, 3, 255};
//#define HALL_SHIFT 1

//KOFORD
uint8_t hallOrder[] = {255, 2, 4, 3, 0, 1, 5, 255};
#define HALL_SHIFT 4
#define SWITCH_FREQ 32000
#define HALL_SAMPLES 10


//-------------------------------------------------------------------

#include "TimerOne.h"
#include "SPI.h"
#include "config.h"

volatile uint32_t lastTime = 0;
volatile uint32_t lastHallTime = 0;
volatile uint32_t curTime = 0;

volatile uint16_t throttle = 0;

void setup(){
  setupPins();
  
  analogWrite(INHA, 0);
  analogWrite(INHB, 0);
  analogWrite(INHC, 0);

  attachInterrupt(HALL1, hallISR, CHANGE);
  attachInterrupt(HALL2, hallISR, CHANGE);
  attachInterrupt(HALL3, hallISR, CHANGE);
}

void hallISR()
{
  uint8_t hall = getHalls();
  uint8_t pos = hallOrder[hall];
  if(pos > 6)
  {
    writeState(255);//error
    return;
  }

  pos = (pos + HALL_SHIFT) % 6;
  //analogWrite(A14, pos * 500 + 500);
  writeState(pos);
  
  lastHallTime = curTime;
}

float avgThrottle = 0;

void loop(){

  curTime = millis();
  float curThrottle = getThrottle();
  
  avgThrottle += (curThrottle - avgThrottle) / 100;
  
  if(curTime - lastTime > 10)
  {
    throttle = avgThrottle * 4095;
    lastTime = curTime;

    if(curTime - lastHallTime > 50)
      hallISR();//calling this randomly somehow causes current spikes to the motor. only call it when needed
  }

  delay(1);
}

uint8_t getHalls()
{
  uint32_t hallCounts[] = {0, 0, 0};
  for(uint32_t i = 0; i < HALL_SAMPLES; i++)
  {
    hallCounts[0] += digitalReadFast(HALL1);
    hallCounts[1] += digitalReadFast(HALL2);
    hallCounts[2] += digitalReadFast(HALL3);
  }

  uint8_t hall = 0;
  if(hallCounts[0] > (HALL_SAMPLES/2))  hall |= 1<<0;
  if(hallCounts[1] > (HALL_SAMPLES/2))  hall |= 1<<1;
  if(hallCounts[2] > (HALL_SAMPLES/2))  hall |= 1<<2;

  return hall;
  
  //uint8_t hall = (digitalReadFast(HALL3) << 2) | (digitalReadFast(HALL2) << 1) | (digitalReadFast(HALL1) << 0);
  //return hall & 0x07;
}


// write the phase to the low side gates
// 1-hot encoding for the phase
// 001 = A, 010 = B, 100 = C
void writeLow(uint8_t phase){
  digitalWriteFast(INLA, (phase&(1<<0)));
  digitalWriteFast(INLB, (phase&(1<<1)));
  digitalWriteFast(INLC, (phase&(1<<2)));
}


void writeState(uint8_t pos)
{
  //Maybe this is necessary? Might solve some problems with bad handshaking?
  //writeHigh(0);
  //writeLow(0);

  if(throttle == 0)
  {
    writeHigh(0b000);
    writeLow(0b000);
    return;
  }

  switch(pos){
    case 0://LOW A, HIGH B
      writeLow(0b001);
      writeHigh(0b010);
      break;
    case 1://LOW A, HIGH C
      writeLow(0b001);
      writeHigh(0b100);
      break;
    case 2://LOW B, HIGH C
      writeLow(0b010);
      writeHigh(0b100);
      break;
    case 3://LOW B, HIGH A
      writeLow(0b010);
      writeHigh(0b001);
      break;
    case 4://LOW C, HIGH A
      writeLow(0b100);
      writeHigh(0b001);
      break;
    case 5://LOW C, HIGH B
      writeLow(0b100);
      writeHigh(0b010);
      break;
  }
}

// write the phase to the high side gates
// 1-hot encoding for the phase
// 001 = A, 010 = B, 100 = C
void writeHigh(uint8_t phase){
  switch(phase){
  case 0b001: // Phase A
    analogWrite(INHB, 0);
    analogWrite(INHC, 0);
    analogWrite(INHA, throttle);
    break;
  case 0b010: // Phase B
    analogWrite(INHA, 0);
    analogWrite(INHC, 0);
    analogWrite(INHB, throttle);
    break;
  case 0b100:// Phase C
    analogWrite(INHA, 0);
    analogWrite(INHB, 0);
    analogWrite(INHC, throttle);
    break;
  default://ALL OFF
    analogWrite(INHA, 0);
    analogWrite(INHB, 0);
    analogWrite(INHC, 0);
  }
}
