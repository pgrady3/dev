#include <i2c_t3.h>

#define LED1 22
#define LED2 21

#define LOAD_SCK 1
#define LOAD_DATA 0

#define HALL 23

#define LOAD_ZERO (15121605.0 + LOAD_SCALE * 0.1)
#define LOAD_SCALE (26159540.0 / 3.0107)  //convert to N*m

int32_t avgLoad = 0;
uint32_t curTime = 0;
uint32_t lastTime = 0;
int32_t avgTime = 100000;

void setup() {
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
    INAinit();
    
    Serial.begin(115200);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    pinMode(HALL, INPUT);
    attachInterrupt(HALL, hallISR, FALLING);

    pinMode(LOAD_SCK, OUTPUT);
    pinMode(LOAD_DATA, INPUT);
    digitalWrite(LOAD_SCK, LOW);
    
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
}

void hallISR()
{
  static uint32_t b = 0;
  digitalWriteFast(LED2, (b++ & 1));
  
  curTime = micros();
  
  int32_t dif = curTime - lastTime;
  
  avgTime += (dif - avgTime) / 8;
  
  lastTime = curTime;
}

float LoadRead()
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

  resp = resp << 8;
  
  avgLoad += (resp - avgLoad) / 10;

  float load = ((float)avgLoad - LOAD_ZERO) / LOAD_SCALE;
  //float load = avgLoad;
  //float load = (float)avgLoad - LOAD_ZERO;
  
  return load;//change to 2's complement
}

void loop() {
  float voltage = INAvoltage();
  float current = INAcurrent();
  float power = voltage * current;
  float omega = 2.0 * 3.1415 / (avgTime * 8) * 1000000;
  float torque = LoadRead();

  Serial.print(omega);
  Serial.print(" ");
  Serial.print(torque);
  Serial.print(" ");
  Serial.print(omega * torque);
  Serial.print(" ");
  Serial.println(power);
  
  while(digitalRead(LOAD_DATA))
    ;

  delay(20);
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


