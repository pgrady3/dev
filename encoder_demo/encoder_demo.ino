#include "SPI.h" 

const uint8_t DO = 12;
const uint8_t CLK = 13;
const uint8_t CSnPin = 10;
const uint8_t A = 4;
const uint8_t B = 3;
const uint8_t I = 2;

volatile uint16_t ticks = 0;
volatile uint8_t a0 = 0;
volatile uint8_t b0 = 0;

void SPI_begin() {
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  SPI.setDataMode(SPI_MODE1);
}

uint16_t SPIread()
{
  digitalWrite(CSnPin, LOW);
  delayMicroseconds(1);
  
  uint8_t d = SPI.transfer(0);
  uint16_t resp = d << 8;
  resp |= SPI.transfer(0);

  digitalWrite(CSnPin, HIGH);

  return resp >> 6;
}

void increment() {
  volatile uint8_t a1 = digitalRead(A);
  volatile uint8_t b1 = digitalRead(B);

  if ((a0 ^ b0) ^ (a0 ^ a1))
    ticks++;
  else
    ticks--;

  a0 = a1;
  b0 = b1;
}

uint16_t readSSI(){
  digitalWrite (CSnPin, LOW);
  delayMicroseconds(1);
  uint16_t value = 0 ;
  for (int i = 0; i < 17; i++)
  {
    digitalWrite (CLK, LOW) ;
    delayMicroseconds(1);
    digitalWrite (CLK, HIGH) ;
    value = (value << 1) | digitalReadFast(DO) ;
    delayMicroseconds(1);
  }
  digitalWrite (CSnPin, HIGH) ;
  uint16_t angle = value >> 6;
  uint8_t statusBits = value & 0x3F;

  //Serial.println(angle);
  // Serial.println(angle, BIN);
  // Serial.println(statusBits, BIN);
  // Serial.println(value, BIN);
  // Serial.println();
  delayMicroseconds(1);
  return angle;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  pinMode(CSnPin,OUTPUT);
  pinMode(CLK,OUTPUT);
  pinMode(DO,INPUT);
  pinMode(I, INPUT);
  pinMode(A, INPUT);
  pinMode(B, INPUT);

  digitalWrite(CSnPin, HIGH);
  digitalWrite(CSnPin, LOW);
  digitalWrite(CSnPin, HIGH);
  attachInterrupt(digitalPinToInterrupt(A), increment, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B), increment, CHANGE);

  SPI_begin();
  
  delay(10);

  ticks = SPIread();
  
}

void loop() {
  ticks = ticks % 1024;
  Serial.print("incremental: ");
  Serial.println(ticks);
  Serial.print("absolute:    ");
  Serial.print(SPIread());
  Serial.print(" ");
  if (digitalRead(I)) {
    Serial.println(1);
  } else {
    Serial.println(); 
  }
  //Serial.println(readSSI());
  Serial.println();
  delay(5);
}
