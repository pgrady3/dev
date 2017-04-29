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
#define WHEEL_CIRC 1.492
#define TICK_DIST (WHEEL_CIRC / 8)

#define WHEEL_TICKS 8
volatile uint32_t tickTimes[WHEEL_TICKS];
volatile uint32_t tickPos;

volatile uint32_t lastHallPulse = 0;
volatile uint32_t lastInaMeasurement = 0;
volatile uint32_t countIntervals = 0;
volatile int32_t avgdT = 1000000;
volatile uint32_t distTicks = 0;

double energyUsed = 0.0;
double distance = 0.0;
double currentSpeed = 0.0;
double temperature = 0.0;
double InaVoltage = 0.0;
double InaCurrent = 0.0;
double InaPower = 0;
double batteryVoltage = 0.0;

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

  pinMode(HALL, INPUT_PULLUP);

  attachInterrupt(HALL, countHallPulse, FALLING);

  myFile = SD.open("data.txt", FILE_WRITE);
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
  InaPower = InaVoltage * InaCurrent;
  
  double currentInaTime = millis();
  energyUsed += InaPower * (currentInaTime - lastInaMeasurement) / 1000;
  lastInaMeasurement = currentInaTime;

  currentSpeed = 1000000.0 / avgdT * WHEEL_CIRC; 
  if(micros() - lastHallPulse > 2000000)
    currentSpeed = 0;
  
  //currentSpeed = avgdT;
  
  /*double average = 0.0;
  
  for (uint8_t i = 0; i < NUMBER_OF_CELLS; i++)
  {
    double result = readCell(i);
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
  }*/

  // 2.74889357 is the circumfence of the wheels.
  distance = distTicks * TICK_DIST;
  

  
  writeToBtSd();

  delay(100);
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
  uint32_t current = micros();

  tickTimes[tickPos++] = current;
  tickPos %= WHEEL_TICKS;

  avgdT = current - tickTimes[tickPos];

  distTicks++;
  
  lastHallPulse = current;

  digitalWrite(LED1, (distTicks) & 1);
}

void writeToBtSd() {
  String outputStr = String(InaVoltage) + " " + String(InaCurrent) + " " + String(InaPower) + " "+ String(currentSpeed) + " " +
                     String(energyUsed) + " " + String(distance) + " " + String(temperature) + " " + 
                     String(batteryOK) + " "+ String(batteryVoltage) +" " + String(millis());
  Serial.println(outputStr);
  Serial2.println(outputStr);
  
  myFile.println(outputStr);
  myFile.flush();
}
