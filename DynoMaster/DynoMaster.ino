#include <i2c_t3.h>

#define LED1 22
#define LED2 21

#define LOAD_SCK 1
#define LOAD_DATA 0

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
    INAinit();
    
    Serial.begin(115200);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    pinMode(LOAD_SCK, OUTPUT);
    pinMode(LOAD_DATA, INPUT);
    digitalWrite(LOAD_SCK, LOW);
    
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
}

int32_t LoadRead()
{
  if(digitalRead(LOAD_DATA))
    return 0;//not ready

  int32_t resp = 0;
  for(uint8_t i = 0; i < 24; i++)
  {
    resp = resp << 1;
    digitalWrite(LOAD_SCK, HIGH);
    delayMicroseconds(1);
    resp |= digitalRead(LOAD_DATA);
    digitalWrite(LOAD_SCK, LOW);
    delayMicroseconds(1);
  }

  digitalWrite(LOAD_SCK, HIGH);
  delayMicroseconds(1);
  digitalWrite(LOAD_SCK, LOW);
  delayMicroseconds(1);

  return resp << 8;//change to 2's complement
}

void loop() {
  Serial.print(INAvoltage());
  Serial.print(" ");
  Serial.print(INAcurrent() * 1000);
  Serial.print(" ");
  Serial.println(LoadRead());
  
  while(digitalRead(LOAD_DATA))
    ;
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


