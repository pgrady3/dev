#include <math.h>

#define ENC_A 1
#define ENC_B 2
#define ENC_CS 0
#define ENC_PROG 3

#define ENC_OFFSET 2.965

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
}

uint16_t ENCread()
{
  digitalWrite(ENC_CS, LOW);
  delayMicroseconds(1);
  
  uint8_t d = SPI.transfer(0);
  uint16_t resp = d << 8;
  resp |= SPI.transfer(0);

  delayMicroseconds(1);
  
  digitalWrite(ENC_CS, HIGH);

  return resp;
}

uint16_t ENCreadAngle()
{
  return ENCread() >> 6;
}

float ENCreadEAngle()
{
  float rot = ENCreadAngle() / 1024.0;
  return fmod(rot * 42.0, 6) + ENC_OFFSET;
}

void ENClinTest()
{
    while(1)
    {
      static uint8_t pos = 0;
      
      hallSetThrottle(0.06);//throttle percent
      writeState(pos%6);
      pos++;
      pos %= 42;

      delay(500);
      
      for(uint8_t i = 0; i < 50; i++)
      {
        Serial.print(pos);
        Serial.print(" ");
        Serial.println(ENCreadAngle());
        delay(10);
      }
    }
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
