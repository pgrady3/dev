#define INHA 6
#define INLA 5
#define INHB 4 
#define INLB 0
#define INHC 3
#define INLC 2

#define THROTTLE 15

#define DRV_EN_GATE 7
#define DRV_CS 10
#define DRV_MOSI 11
#define DRV_MISO 12
#define DRV_CLK 13

#define FAULT 9
#define ISENSE1 23

#define LED 13


#define MAX_THROTTLE  1000
#define MIN_THROTTLE  300

uint16_t SPIread(uint8_t addr);
void SPIwrite(uint8_t addr, uint16_t data);
void setupPins();
float getThrottle();

float getThrottle()
{
  uint16_t rawThrottle = analogRead(THROTTLE);
  float throttle = (rawThrottle - MIN_THROTTLE) / (float)(MAX_THROTTLE - MIN_THROTTLE);
  
  if(throttle > 1)
    throttle = 1;
  if (throttle < 0)
    throttle = 0;

  return throttle;
}

void setupPins()
{
  //pinMode(LED, OUTPUT);
  pinMode(FAULT, INPUT);
  
  pinMode(INHA, OUTPUT);
  pinMode(INLA, OUTPUT);
  pinMode(INHB, OUTPUT);
  pinMode(INLB, OUTPUT);
  pinMode(INHC, OUTPUT);
  pinMode(INLC, OUTPUT);
  
  pinMode(THROTTLE, INPUT);

  pinMode(DRV_EN_GATE, OUTPUT);

  pinMode(DRV_CLK, OUTPUT);
  pinMode(DRV_MOSI, OUTPUT);
  pinMode(DRV_MISO, INPUT);
  pinMode(DRV_CS, OUTPUT);
  digitalWriteFast(DRV_CS, HIGH);
  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE1);
  
  pinMode(ISENSE1, INPUT);

  // change the analog write frequency to 8 kHz
  analogWriteFrequency(INHA, 8000);
  analogWriteFrequency(INHB, 8000);
  analogWriteFrequency(INHC, 8000);
  analogWriteResolution(12); // write from 0 to 2^12 = 4095

  analogWrite(INHA, 0);
  analogWrite(INHB, 0);
  analogWrite(INHC, 0); 
  
  Serial.begin(115200);

  digitalWriteFast(DRV_EN_GATE, LOW);
  delay(10);
  digitalWriteFast(DRV_EN_GATE, HIGH);
  delay(10);

  while(SPIread(0x01) != 0x01)
  {
    Serial.println("DRV init fail");
    delay(500);
  }

  //SPI.end();
}

uint16_t SPIread(uint8_t addr)
{
  digitalWrite(DRV_CS, LOW);

  delayMicroseconds(50);
  uint8_t d = 1 << 7;
  d |= addr << 3;
  SPI.transfer(d);
  SPI.transfer(0);

  digitalWrite(DRV_CS, HIGH);
  delayMicroseconds(30);
  digitalWrite(DRV_CS, LOW);
  
  d = SPI.transfer(1<<7);
  uint16_t resp = d << 8;
  resp |= SPI.transfer(0);

  digitalWrite(DRV_CS, HIGH);

  return resp & 0x7FF;
}

void SPIwrite(uint8_t addr, uint16_t data)
{
  digitalWriteFast(DRV_CS, LOW);

  delayMicroseconds(50);
  uint8_t d = data >> 8;
  d |= addr << 3;
  SPI.transfer(d);
  d = data & 0xFF;
  SPI.transfer(d);
    
  digitalWriteFast(DRV_CS, HIGH);
}
