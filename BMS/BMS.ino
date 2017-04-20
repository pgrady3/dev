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
#define BUFFER_LENGTH 64

uint8_t head = 0;
uint32_t lastHallPulse = 0;
uint32_t lastInaMeasurement = 0;
uint32_t countIntervals = 0;
double energyUsed = 0.0;
double distance = 0.0;
double currentSpeed = 0.0;
double temperature = 0.0;
double InaVoltage = 0.0;
double InaCurrent = 0.0;
double batteryVoltage = 0.0;
double power = 0.0;

bool batteryOK = true;

uint32_t intervalBuffer[BUFFER_LENGTH];

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

  for (int i = 0; i < BUFFER_LENGTH; i++) {
    intervalBuffer[i] = 0;
  }

  attachInterrupt(digitalPinToInterrupt(HALL), countHallPulse, FALLING);

  lastInaMeasurement = millis();
}

double readCell(uint8_t cell)
{
  digitalWrite(S0, cell & 1);
  cell = cell >> 1;
  digitalWrite(S1, cell & 1);
  cell = cell >> 1;
  digitalWrite(S2, cell & 1);
  cell = cell >> 1;
  digitalWrite(S3, cell & 1);
  delayMicroseconds(1000);
  double volt = (double)analogRead(14) / 1024 * 3.3 / 2.2 * 26.2;
  return volt;
}

void loop() {

  InaVoltage = INAvoltage();
  InaCurrent = INAcurrent();
  power = InaVoltage * InaCurrent;

  double currentInaTime = millis();
  energyUsed += (InaVoltage * InaCurrent) * ((double)(currentInaTime - lastInaMeasurement) / 1000.0);
  lastInaMeasurement = currentInaTime;

  double average = 0.0;
  for (uint8_t i = 0; i < NUMBER_OF_CELLS; i++)
  {
    double result = readCell(i);
    average += result;
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
  distance = ((double)countIntervals) / 8.0 * 2.74889357;
  //Unit is now m/s.
  currentSpeed = getSpeed();
  writeToBtSd();
}

double INAcurrent()
{
  int16_t raw = INAreadReg(0x01); //deliberate bad cast! the register is stored as two's complement
  return raw * 0.0000025 / 0.001 ; //2.5uV lsb and 1mOhm resistor
}

double INAvoltage()
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
    writeBuffer(thisInterval);
  }
  lastHallPulse = current;
}

void writeToBtSd() {
  String outputStr = String(InaVoltage) + " " + String(InaCurrent) + " " + String(power) + " " +
                     String(currentSpeed) + " " +
                     String(energyUsed) + " " + String(distance) + " " + String(temperature) + " " +
                     String(batteryOK) + " " + String(batteryVoltage) + " " + String(millis()) + " ";
  Serial.println(outputStr);
  Serial2.println(outputStr);
  myFile = SD.open("data.txt", FILE_WRITE);
  myFile.println(outputStr);
  myFile.close();
}

double bufferMean() {
  double sum = 0.0;
  double counts = 0.0;
  for (int i = 0; i < BUFFER_LENGTH; i++) {
    if (intervalBuffer[i] != 0) {
      sum += (double)intervalBuffer[i];
      counts += 1.0;
    }
  }
  if (counts < 0.01) {
    return 0;
  }
  return sum / counts;
}

void writeBuffer(uint32_t data) {
  intervalBuffer[head] = data;
  if (head + 1 == BUFFER_LENGTH) {
    head = 0;
  } else {
    head++;
  }
}

double getSpeed() {
  double averageInterval = bufferMean();
  if (averageInterval == 0) {
    return 0.0;
  }
  // Assume we have 8 intervals for one revolution.
  double radianSpeed = (360.0 / 8.0) / (averageInterval / 1000.0);
  return (radianSpeed / 360) * 2.74889357;
}



