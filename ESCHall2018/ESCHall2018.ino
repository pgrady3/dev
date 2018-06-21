#include <i2c_t3.h>
#include "TimerOne.h"
#include "SPI.h"
#include "config.h"

/*
uint8_t hallOrder[] = {255, 5, 3, 4, 1, 0, 2, 255}; //for gemini hub motor
#define HALL_SHIFT 1*/

uint8_t hallOrder[] = {255, 1, 3, 2, 5, 0, 4, 255}; //for maxwell motor
#define HALL_SHIFT 2
#define HALL_SAMPLES 10

uint32_t lastTime = 0;

#define LOG_SAMPLES 6000
volatile uint32_t logI1[LOG_SAMPLES];
volatile uint32_t logI2[LOG_SAMPLES];
int32_t posI = -1;

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
  writeState(pos);
}

void loop(){
    
  
  uint32_t curTime = millis();

  /*uint32_t i1 = analogRead(ISENSE1);
  uint32_t i2 = analogRead(ISENSE2);
  if(posI < 0 && i1 > 700)
    posI = 0;

  if(posI >= 0 && posI < LOG_SAMPLES)
  {
    logI1[posI] = i1;
    logI2[posI++] = i2;
  }

  if(posI == LOG_SAMPLES)
  {
    posI++;
    throttle = 0;
    for(uint32_t i = 0; i < LOG_SAMPLES; i++)
    {
      Serial.print(logI1[i]);
      Serial.print(' ');
      Serial.println(logI2[i]);
      delay(1);
    }
  }*/
  
  if(curTime - lastTime > 50)
  {
    volatile uint16_t driverThrottle = getThrottle() * 4095;
    if(curTime - BMSMillis < 300)//less than 300ms since BMS update
    {
      //digitalWrite(LED2, HIGH);
      if(BMSThrottle == 0)
        throttle = driverThrottle;
      else
        throttle = BMSThrottle / 16;//scale from full 16 bit to 12 bit
    }
    else
    {
      //digitalWrite(LED2, LOW);
      throttle = driverThrottle;
    }
    
    hallISR();
    
    lastTime = curTime;
    Serial.print(driverThrottle);
    Serial.print(" ");
    Serial.print(analogRead(THROTTLE));
    Serial.print(" ");
    Serial.println(throttle);
  }

  delayMicroseconds(100);
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
  
  if(hall == 7)
    digitalWrite(LED1, HIGH);
  else
    digitalWrite(LED1, LOW);
  
  return hall & 0x07;
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
