#include <i2c_t3.h>
#include "TimerOne.h"
#include "SPI.h"
#include "config.h"
#include "pwm.h"

uint8_t hallOrder[] = {255, 1, 3, 2, 5, 0, 4, 255}; //for maxwell motor
#define HALL_SHIFT 2
#define HALL_SAMPLES 10

uint32_t lastLoopTime = 0;
volatile uint32_t lastHallTime = 0;

#define LOG_SAMPLES 2000
volatile uint16_t logI1[LOG_SAMPLES];
volatile uint16_t logI2[LOG_SAMPLES];
volatile uint16_t logZcA[LOG_SAMPLES];
volatile uint16_t logZcB[LOG_SAMPLES];
volatile uint16_t logZcC[LOG_SAMPLES];
volatile uint8_t logHall[LOG_SAMPLES];
volatile uint32_t logTime[LOG_SAMPLES];
volatile int32_t posI = -1;

volatile uint16_t throttle = 0;

void setup(){
  setupPins();
  PWMInit();

  attachInterrupt(HALL1, hallISR, CHANGE);
  attachInterrupt(HALL2, hallISR, CHANGE);
  attachInterrupt(HALL3, hallISR, CHANGE);
}

void hallISR()
{
  uint8_t hall = getHalls();
  uint8_t pos = hallOrder[hall];
  lastHallTime = millis();
  if(pos > 6)
  {
    PWMSetMotorPos(255);//error
    return;
  }

  pos = (pos + HALL_SHIFT) % 6;
  PWMSetMotorPos(pos);
}

void loop(){
    
  
  uint32_t curTime = millis();

  uint16_t i1 = analogRead(ISENSE1);
  uint16_t i2 = analogRead(ISENSE2);
  uint16_t zcA = analogRead(ZC_A);
  uint16_t zcB = analogRead(ZC_B);
  uint16_t zcC = analogRead(ZC_C);
  uint8_t hP = hallOrder[getHalls()];
  uint32_t us = micros();
  
  if(posI < 0 && throttle > 500)
    posI = 0;

  if(posI >= 0 && posI < LOG_SAMPLES)
  {
    logHall[posI] = hP; 
    logTime[posI] = us;
    logZcA[posI] = zcA;
    logZcB[posI] = zcB;
    logZcC[posI] = zcC;
    logI1[posI] = i1;
    logI2[posI++] = i2;
    
  }

  if(posI == LOG_SAMPLES)
  {
    posI++;
    throttle = 0;
    for(uint32_t i = 0; i < LOG_SAMPLES; i++)
    {
      Serial.print(logTime[i]);
      Serial.print(' ');
      Serial.print(logHall[i]);
      Serial.print(' ');
      Serial.print(logZcA[i]);
      Serial.print(' ');
      Serial.print(logZcB[i]);
      Serial.print(' ');
      Serial.print(logZcC[i]);
      Serial.print(' ');
      Serial.print(logI1[i]);
      Serial.print(' ');
      Serial.println(logI2[i]);
      delay(1);
    }
  }
  
  if(curTime - lastLoopTime > 50)
  {
    volatile uint16_t driverThrottle = getThrottle() * 255;
    if(curTime - BMSMillis < 300)//less than 300ms since BMS update
    {
      //digitalWrite(LED2, HIGH);
      if(BMSThrottle == 0)
        PWMSetDuty(driverThrottle);
      else
        PWMSetDuty(BMSThrottle / 256);//scale from full 16 bit to 8 bit
    }
    else
    {
      //digitalWrite(LED2, LOW);
      PWMSetDuty(driverThrottle);
    }

    if(curTime - lastHallTime > 100)
      hallISR();
    
    lastLoopTime = curTime;
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
