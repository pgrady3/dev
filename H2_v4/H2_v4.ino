#include <i2c_t3.h>
#include "INA.h"

#define LED1 10
#define LED2 11
#define LED3 13

#define SUPPLY_VALVE 5
#define PURGE_VALVE 6
#define FAN 4

#define BUZZER 9
#define SHORT_CIRCUIT 8
#define PASS 3

void setup() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Serial.begin(115200);
  Serial1.begin(57600);
  UART0_C3 = 16;//tx invert
  UART0_S2 = 16;//rx invert


  
  INAinit();
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  pinMode(PASS, OUTPUT);
  digitalWrite(PASS, HIGH);
  pinMode(SHORT_CIRCUIT, OUTPUT);
  digitalWrite(SHORT_CIRCUIT, LOW);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  pinMode(SUPPLY_VALVE, OUTPUT);
  digitalWrite(SUPPLY_VALVE, HIGH);
  pinMode(PURGE_VALVE, OUTPUT);
  digitalWrite(PURGE_VALVE, LOW);
  pinMode(FAN, OUTPUT);
  digitalWrite(FAN, LOW);
}

void loop() {
  digitalWrite(LED2, digitalRead(LED1));
  digitalWrite(LED1, !digitalRead(LED1));

  Serial.print(INAvoltage(), 4);
  Serial.print(" ");
  Serial.println(INAcurrent(), 4);
  delay(200);
  
  Serial1.print("A\r");
  while(Serial1.available())
  {
    Serial.print((char)Serial1.read());
  }
  Serial.println();
}

void FCShort(uint32_t duration)
{
  digitalWrite(PASS, LOW);
  delay(1);
  digitalWrite(SHORT_CIRCUIT, HIGH);
  delay(duration);
  digitalWrite(SHORT_CIRCUIT, LOW);
  delay(2);
  digitalWrite(PASS, HIGH);
}

