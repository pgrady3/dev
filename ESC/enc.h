#include <math.h>

#define ENC_A 1
#define ENC_B 2
#define ENC_CS 0
#define ENC_PROG 3

#define ENC_OFFSET 25806

uint16_t curAngle = 0;
uint32_t encTicks = 0;

void encISRA();
void encISRB();
void ENCgetAbsAngle();

void ENCinit()
{
  pinMode(ENC_PROG, OUTPUT);
  digitalWrite(ENC_PROG, LOW);
  pinMode(ENC_CS, OUTPUT);
  digitalWriteFast(ENC_CS, HIGH);
  digitalWriteFast(ENC_CS, LOW);
  digitalWriteFast(ENC_CS, HIGH);

  delay(100);//not sure if we need this

  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);

  attachInterrupt(ENC_A, encISRA, CHANGE);
  attachInterrupt(ENC_B, encISRB, CHANGE);

  NVIC_SET_PRIORITY(IRQ_PORTB, 0); // Zero = highest priority
  NVIC_SET_PRIORITY(IRQ_PORTD, 0); // Zero = highest priority
}

void encISRA()
{
  digitalWrite(LED1, HIGH);
  uint8_t a = digitalRead(ENC_A);
  uint8_t b = digitalRead(ENC_B);
  
  if(a ^ b)
    curAngle--;
  else
    curAngle++;

  digitalWrite(LED1, LOW);
}

void encISRB()
{
  digitalWrite(LED1, HIGH);
  uint8_t a = digitalRead(ENC_A);
  uint8_t b = digitalRead(ENC_B);
  
  if(a ^ b)
    curAngle++;
  else
    curAngle--;

  digitalWrite(LED1, LOW);
}

uint16_t ENCread()
{
  digitalWrite(ENC_CS, LOW);
  delayMicroseconds(1);
  
  uint8_t d = SPI.transfer(0);
  uint16_t resp = d << 8;
  resp |= SPI.transfer(0);
  
  digitalWrite(ENC_CS, HIGH);

  return resp;
}

void ENCgetAbsAngle()
{
  uint16_t newAngle = ENCread() >> 6;
  //Serial.println(newAngle - curAngle);
  curAngle = newAngle;
}

uint16_t ENCreadAngle()
{
  curAngle = curAngle & 0x3FF;
  encTicks++;
  return curAngle;//returns 10 bit value
}

uint16_t ENCreadEAngle()
{
  uint32_t rot = (uint32_t)(ENCreadAngle() * 64 * 7) - ENC_OFFSET;//scale so one e-rev is 65536
  return rot;//effective modulo in the cast down to 16 bit
}


void ENCvarTest()
{
    pinMode(ENC_PROG, OUTPUT);
    digitalWrite(ENC_CS, HIGH);
    digitalWrite(ENC_PROG, HIGH);
    delay(1);
    digitalWrite(ENC_CS, LOW);
    delay(1);
    digitalWrite(ENC_PROG, LOW);

    while(1)
    {
      static uint8_t pos = 0;
      
      hallSetThrottle(0.07);//throttle percent
      writeState(pos++);
      pos %= 6;
      
      Serial.println(ENCread()>>6);
      delay(13);
    }
}
