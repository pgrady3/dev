#define HALL1 16
#define HALL2 17
#define HALL3 20

uint8_t hallOrder[] = {255, 5, 1, 0, 3, 4, 2, 255};
#define HALL_SHIFT 0

void hallISR();
uint8_t hallGet();
void writeState(uint8_t pos);
void writeHigh(uint8_t phase);

volatile uint16_t curThrottle = 0;
uint32_t lastTime = 0;

void hallSetThrottle(float pos)
{
 if(pos < 0)  pos = 0;
 if(pos > 1)  pos = 1;

 curThrottle = pos * 4095;
}

void hallInit()
{
  pinMode(HALL1, INPUT);
  pinMode(HALL2, INPUT);
  pinMode(HALL3, INPUT);
  
  attachInterrupt(HALL1, hallISR, CHANGE);
  attachInterrupt(HALL2, hallISR, CHANGE);
  attachInterrupt(HALL3, hallISR, CHANGE);

   // change the analog write frequency to 8 kHz
  analogWriteFrequency(INHA, 8000);
  analogWriteFrequency(INHB, 8000);
  analogWriteFrequency(INHC, 8000);
  analogWriteResolution(12); // write from 0 to 2^12 = 4095

  analogWrite(INHA, 0);
  analogWrite(INHB, 0);
  analogWrite(INHC, 0); 
}

void hallPoll()
{
  uint32_t curTime = millis();
  if(curTime - lastTime > 50)
  {
    hallSetThrottle(getThrottle());
    hallISR();
    lastTime = curTime;
  }
}

void hallISR()
{
  uint8_t hall = hallGet();
  uint8_t pos = hallOrder[hall];
  if(pos > 6)
  {
    writeState(255);//error
    return;
  }

  pos = (pos + HALL_SHIFT) % 6;
  writeState(pos);
}

uint8_t hallGet()
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

// write the phase to the high side gates
// 1-hot encoding for the phase
// 001 = A, 010 = B, 100 = C
void writeHigh(uint8_t phase){
  switch(phase){
  case 0b001: // Phase A
    analogWrite(INHB, 0);
    analogWrite(INHC, 0);
    analogWrite(INHA, curThrottle);
    break;
  case 0b010: // Phase B
    analogWrite(INHA, 0);
    analogWrite(INHC, 0);
    analogWrite(INHB, curThrottle);
    break;
  case 0b100:// Phase C
    analogWrite(INHA, 0);
    analogWrite(INHB, 0);
    analogWrite(INHC, curThrottle);
    break;
  default://ALL OFF
    analogWrite(INHA, 0);
    analogWrite(INHB, 0);
    analogWrite(INHC, 0);
  }
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
