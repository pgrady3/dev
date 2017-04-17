#define INHA 23
#define INLA 4
#define INHB 22 
#define INLB 5
#define INHC 21
#define INLC 6

#define THROTTLE 15

#define DRV_EN_GATE 7
#define DRV_CS 10
#define DRV_MOSI 11
#define DRV_MISO 12
#define DRV_CLK 13

#define ISENSE1 17
#define ISENSE2 16
#define VSENSE 14

#define LED1 9
#define LED2 8

#define MAX_THROTTLE  1000
#define MIN_THROTTLE  300

uint16_t DRVread(uint8_t addr);
void DRVwrite(uint8_t addr, uint16_t data);
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

void init()
{
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
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
  pinMode(ISENSE2, INPUT);

  Serial.begin(115200);

  digitalWriteFast(DRV_EN_GATE, LOW);
  delay(10);
  digitalWriteFast(DRV_EN_GATE, HIGH);
  delay(10);

  while(DRVread(0x01) != 0x01)
  {
    Serial.print("DRV init fail. 0x00=");
    Serial.print(DRVread(0x00));
    Serial.print(" 0x01=");
    Serial.println(DRVread(0x01));
    
    digitalWrite(LED1, !digitalRead(LED1));
    digitalWrite(LED2, !digitalRead(LED2));

    digitalWrite(DRV_EN_GATE, LOW);
    delay(10);
    digitalWrite(DRV_EN_GATE, HIGH);
    delay(10);
  }

  DRVwrite(0x02,  (1<<3) | (10<<6));//Set 3PWM inputs, medium current limit
  
  //SPI.end();
}

uint16_t DRVread(uint8_t addr)
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

void DRVwrite(uint8_t addr, uint16_t data)
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
