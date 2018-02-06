#include "TimerOne.h"
#include "SPI.h"
#include "config.h"

uint8_t hallOrder[] = {255, 5, 1, 0, 3, 4, 2, 255};
#define HALL_SHIFT 0

#define ISENSE_LEN  200
uint32_t isenseHistory[ISENSE_LEN];
uint32_t isensePos = 0;
uint32_t lastTime = 0;

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
  uint32_t maxIsense = 0;
  for(uint32_t i = 0; i < ISENSE_LEN; i++)
  {
    if(isenseHistory[i] > maxIsense)
      maxIsense = isenseHistory[i];
  }
  
  uint32_t curTime = millis();
  if(curTime - lastTime > 50)
  {
    throttle = getThrottle() * 4095;
    hallISR();

    float current = (maxIsense - 468.0) / 40.0;
    Serial.println(current);

    lastTime = curTime;
  }
  
  isenseHistory[isensePos++] = analogRead(ISENSE1);
  isensePos %= ISENSE_LEN;

  
  /*if(digitalRead(FAULT) == LOW)
  {
    throttle = 0;
    SPIwrite(0x02, 0x04);//reset all
    delay(1);
    digitalWriteFast(DRV_EN_GATE, LOW);
    delay(1);
    digitalWriteFast(DRV_EN_GATE, HIGH);
    delay(5);
    Serial.println("Reset gate");
  }

  Serial.println(SPIread(0x00), HEX);
  Serial.println(SPIread(0x01), HEX);
  Serial.println();*/

  delay(1);
}

uint8_t getHalls()
{
  uint8_t hall = (digitalReadFast(HALL3) << 2) | (digitalReadFast(HALL2) << 1) | (digitalReadFast(HALL1) << 0);
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
  writeHigh(0);
  writeLow(0);

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
