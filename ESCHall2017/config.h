#define INHA 23
#define INLA 4
#define INHB 22 
#define INLB 5
#define INHC 21
#define INLC 6

#define HALL1 1
#define HALL2 2
#define HALL3 3

#define THROTTLE 15

#define DRV_EN_GATE 7
#define DRV_CS 10
#define DRV_MOSI 11
#define DRV_MISO 12
#define DRV_CLK 13

#define ISENSE1 17
#define ISENSE2 16

#define LED1 9
#define LED2 8


#define MAX_THROTTLE  1000
#define MIN_THROTTLE  300

volatile uint8_t reg = 0;
volatile uint16_t BMSThrottle = 0;
volatile uint32_t BMSMillis = 0;

uint16_t SPIread(uint8_t addr);
void SPIwrite(uint8_t addr, uint16_t data);
void setupPins();
float getThrottle();
void receiveEvent(size_t count);
void requestEvent(void);

void receiveEvent(size_t count)
{
  reg = Wire.readByte();

  if(reg == 0x40)//set throttle
  {
    BMSThrottle = Wire.readByte() << 8;
    BMSThrottle |= Wire.readByte();
    
    BMSMillis = millis();
  }
}


void requestEvent(void)
{
  uint8_t data = 0;
  
  if(reg == 0x12)
    data = 0x34;
    
  Wire.write(&data, 1);
}

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
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  //pinMode(FAULT, INPUT);
  
  pinMode(INHA, OUTPUT);
  pinMode(INLA, OUTPUT);
  pinMode(INHB, OUTPUT);
  pinMode(INLB, OUTPUT);
  pinMode(INHC, OUTPUT);
  pinMode(INLC, OUTPUT);
  
  pinMode(HALL1, INPUT);
  pinMode(HALL2, INPUT);
  pinMode(HALL3, INPUT);
  
  pinMode(THROTTLE, INPUT);

  pinMode(DRV_EN_GATE, OUTPUT);

  pinMode(DRV_CLK, OUTPUT);
  pinMode(DRV_MOSI, OUTPUT);
  pinMode(DRV_MISO, INPUT);
  pinMode(DRV_CS, OUTPUT);
  digitalWriteFast(DRV_CS, HIGH);

  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  SPI.setDataMode(SPI_MODE1);

  Wire.begin(I2C_SLAVE, 0x66, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);//initialize i2c
  Wire.onReceive(receiveEvent);//setup events
  Wire.onRequest(requestEvent);
  
  pinMode(ISENSE1, INPUT);
  pinMode(ISENSE2, INPUT);

  // change the analog write frequency to 8 kHz
  analogWriteFrequency(INHA, 8000);
  analogWriteFrequency(INHB, 8000);
  analogWriteFrequency(INHC, 8000);
  analogWriteResolution(12); // write from 0 to 2^12 = 4095

  Serial.begin(115200);

  digitalWriteFast(DRV_EN_GATE, LOW);
  delay(10);
  digitalWriteFast(DRV_EN_GATE, HIGH);
  delay(10);

  /*while(SPIread(0x01) != 0x01)
  {
    Serial.println("DRV init fail");
    SPIwrite(0x02, 0x03);
    for(uint32_t i = 0; i < 4; i++)
    {
      Serial.println(SPIread(i));
      Serial.println(SPIread(i));
    }
    
    digitalWriteFast(DRV_EN_GATE, LOW);
    delay(10);
    digitalWriteFast(DRV_EN_GATE, HIGH);
    delay(500);
  }*/

  //SPI.end();
}

uint16_t SPIread(uint8_t addr)
{
  delayMicroseconds(50);
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
