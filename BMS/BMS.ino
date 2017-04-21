#include <i2c_t3.h>
#include <SD.h>

#define LED1 3
#define LED2 21

#define S0 15
#define S1 16
#define S2 17
#define S3 20

#define RELAY 2
#define HALL 23
#define SD_CS 8

#define NUMBER_OF_CELLS 16
#define CELL_MIN 2.7
#define CELL_MAX 4.2
#define MAX_TEMPERATURE 50.0

uint32_t lastHallPulse = 0;
uint32_t lastInaMeasurement = 0;
uint32_t countIntervals = 0;
float energyUsed = 0.0;
float distance = 0.0;
float currentSpeed = 0.0;
float temperature = 0.0;
float InaVoltage = 0.0;
float InaCurrent = 0.0;
float InaPower = 0;

float batteryVoltage = 0.0;
bool batteryOK = true;

File myFile;

void setup() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  INAinit();

  Serial.begin(115200);
  Serial2.begin(9600);
  SD.begin(SD_CS);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(RELAY, OUTPUT);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  digitalWrite(RELAY, HIGH);

  attachInterrupt(digitalPinToInterrupt(HALL), countHallPulse, FALLING);
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

  InaVoltage = INAvoltage();
  InaCurrent = INAcurrent();
  InaPower = InaVoltage * InaCurrent;
  
  float currentInaTime = millis();
  energyUsed += InaPower * (currentInaTime - lastInaMeasurement) / 1000;
  lastInaMeasurement = currentInaTime;
  /*float average = 0.0;
  
  for (uint8_t i = 0; i < NUMBER_OF_CELLS; i++)
  {
    float result = readCell(i);
    average += result;
    cellVolts[i] = result;
    if ((result < CELL_MIN) || (result > CELL_MAX)) {
      batteryOK = false;
    }
  }
  batteryVoltage = average / NUMBER_OF_CELLS;

  //TODO: measure temperature here.
  if (temperature > MAX_TEMPERATURE) {
    batteryOK = false;
  }

  // 2.74889357 is the circumfence of the wheels.
  distance = ((float)countIntervals) / 8.0 * 2.74889357;*/

  //TODO: calculate speed here.

  // Not sure where to put the write function.
  writeToBtSd();

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
  if (Wire.available() < 2)
    return 0;

  uint16_t resp = (uint16_t)Wire.read() << 8;
  resp |= Wire.read();

  return resp;
}

void countHallPulse() {
  uint32_t current = millis();
  uint32_t thisInterval = current - lastHallPulse;
  if (thisInterval < 1000) {
    countIntervals++;
  }
  lastHallPulse = current;
}

void writeToBtSd() {
  String outputStr = String(InaVoltage) + " " + String(InaCurrent) + " " + String(currentSpeed) + " " +
                     String(energyUsed) + " " + String(distance) + " " + String(temperature) + " " + 
                     String(batteryOK) + " "+ String(batteryVoltage) + String(millis()) + " ";
  Serial.println(outputStr);
  Serial2.println(outputStr);
  /*myFile = SD.open("data.txt", FILE_WRITE);
  myFile.println(outputStr);
  myFile.close();*/
}

