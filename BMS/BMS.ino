#include <i2c_t3.h>
#include <SD.h>
#include "Adafruit_GPS.h"
#include "INA.h"
#include "H2.h"

#define LED1 3
#define LED2 21

#define S0 15
#define S1 16
#define S2 17
#define S3 20

#define RELAY 2
#define SOLENOID 7
#define HALL 23
#define SD_CS 8
#define H2_SENSOR 22


#define NUMBER_OF_CELLS 16
#define CELL_MIN 2.7
#define CELL_MAX 4.2
#define MAX_TEMPERATURE 50.0

#define WHEEL_CIRC 1.492
#define WHEEL_TICKS 8
#define TICK_DIST (WHEEL_CIRC / WHEEL_TICKS)

volatile uint32_t tickTimes[WHEEL_TICKS];
volatile uint32_t tickPos;

volatile uint32_t loopTime = 0;
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
uint32_t h2Detected = 0;

File myFile;
Adafruit_GPS GPS(&Serial1);

void setup() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  //Wire.setDefaultTimeout(1000);
  INAinit();

  Serial.begin(115200);
  Serial2.begin(38400);
  SD.begin(SD_CS);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(RELAY, OUTPUT);
  pinMode(SOLENOID, OUTPUT);
  digitalWrite(RELAY, HIGH);
  digitalWrite(SOLENOID, HIGH);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  pinMode(HALL, INPUT_PULLUP);

  attachInterrupt(HALL, countHallPulse, FALLING);

  myFile = SD.open("data.txt", FILE_WRITE);

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_BAUD_57600);
  delay(1000);
  Serial1.end();
  
  delay(1000);
  GPS.begin(57600);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);   // 1 Hz update rate
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
  while(GPS.read());//parse all GPS chars available

  if (GPS.newNMEAreceived())
    GPS.parse(GPS.lastNMEA());

  
  uint32_t curTime = millis();
  if(curTime < loopTime + 100)//if less than 100ms, start over
    return;
  
  loopTime = curTime;

  InaVoltage = INAvoltage();
  InaCurrent = INAcurrent();
  InaPower = InaVoltage * InaCurrent;
  
  double currentInaTime = millis();
  energyUsed += InaPower * (currentInaTime - lastInaMeasurement) / 1000;
  lastInaMeasurement = currentInaTime;

  currentSpeed = 1000000.0 / avgdT * WHEEL_CIRC; 
  if(micros() - lastHallPulse > 2000000)
    currentSpeed = 0;
  
  distance = distTicks * TICK_DIST;
  
  writeToBtSd();
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
  uint32_t startMicros = micros();
  
  String outputStr = String(InaVoltage, 3) + " " + String(InaCurrent, 3) + " " + String(InaPower) + " "+ String(currentSpeed) + " " +
                     String(energyUsed) + " " + String(distance) + " " + String(temperature) + " " + 
                     String(batteryOK) + " "+ String(batteryVoltage) +" " + String(millis());// + " " + String(GPS.latitude, 4) + String(GPS.lat) + 
                     //" " + String(GPS.longitude, 4) + String(GPS.lon) + " " + String(GPS.satellites);
  
  
  Serial.println(outputStr);
  
  Serial2.println(outputStr);

  //Serial.println(micros() - startMicros);
  
  myFile.println(outputStr);
  myFile.flush();
  
  
}
