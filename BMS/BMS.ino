#include <i2c_t3.h>

#define LED1 3
#define LED2 21

#define S0 15
#define S1 16
#define S2 17
#define S3 20

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
    INAinit();
    
    Serial.begin(115200);
    Serial2.begin(9600);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);

    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
}

float readCell(uint8_t cell)
{
  digitalWrite(S0, cell & 1);
  cell = cell >> 1;
  digitalWrite(S1, cell & 1);
  cell = cell >> 1;
  digitalWrite(S2, cell & 1);
  cell = cell >> 1;
  digitalWrite(S3, cell & 1);

  delayMicroseconds(1000);
  float volt = (float)analogRead(14) / 1024 * 3.3 / 2.2 * 26.2;

  return volt;
}

void loop() {
  Serial.print(INAvoltage());
  Serial.print(" ");
  Serial.println(INAcurrent() * 1000);

  Serial2.print(INAvoltage());
  Serial2.print(" ");
  Serial2.println(INAcurrent() * 1000);

  for(uint8_t i = 0; i < 16; i++)
  {
    Serial.print(readCell(i));
    Serial.print(" ");
  }

  Serial.println();
  
  delay(100);
}

float INAcurrent()
{
  int16_t raw = INAreadReg(0x01); //deliberate bad cast! the register is stored as two's complement
  return raw * 0.0000025 / 0.001 ; //2.5uV lsb and 1mOhm resistor
}

float INAvoltage()
{
  uint16_t raw = INAreadReg(0x02);
  return raw * 0.00125; //multiply by 1.25mV LSB 
}

void INAinit()
{
  Wire.beginTransmission(0x40);
  Wire.write(0x00);//reg select = 0x00
  Wire.write(0b0111);//64 averages, 1ms voltage sampling
  Wire.write(0b100111);//1ms current sampling, free running
  Wire.endTransmission();
}

uint16_t INAreadReg(uint8_t reg)
{
  Wire.beginTransmission(0x40);
  Wire.write(reg);//read from the bus voltage
  Wire.endTransmission();

  Wire.requestFrom(0x40, 2);

  delayMicroseconds(100);
  if(Wire.available() < 2)
    return 0;

  uint16_t resp = (uint16_t)Wire.read() << 8;
  resp |= Wire.read();

  return resp;
}

