#include <i2c_t3.h>
#include "INA.h"

#define LED1 10
#define LED2 11
#define LED3 13

#define BUZZER 9
#define SHORT_CIRCUIT 8
#define PASS 3

void setup() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Serial.begin(115200);

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
}

void loop() {
  digitalWrite(LED2, digitalRead(LED1));
  digitalWrite(LED1, !digitalRead(LED1));

  Serial.print(INAvoltage());
  Serial.print(" ");
  Serial.println(INAcurrent());
  delay(200);
  //FCShort(50);
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

