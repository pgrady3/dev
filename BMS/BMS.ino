#include <i2c_t3.h>
#include <SD.h>
#include "Adafruit_GPS.h"
#include "INA.h"
#include "H2.h"
#include "ms5611.h"

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
#define TEMP 22

#define WHEEL_CIRC 1.492
#define WHEEL_TICKS 8
#define TICK_DIST (WHEEL_CIRC / WHEEL_TICKS)

#define TARGET_CURRENT 6

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
double startingAlt = 0;
double currentAlt = 0;
double throttle = 0;

bool batteryOK = true;
uint32_t h2Detected = 0;

File myFile;
Adafruit_GPS GPS(&Serial1);
MS5611 baro;

void setup() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  //Wire.setDefaultTimeout(1000);
  INAinit();

  Serial.begin(115200);
  Serial2.begin(115200);
  SD.begin(SD_CS);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(RELAY, OUTPUT);
  pinMode(SOLENOID, OUTPUT);
  pinMode(TEMP, INPUT);
  digitalWrite(RELAY, HIGH);
  digitalWrite(SOLENOID, HIGH);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  pinMode(HALL, INPUT_PULLUP);

  attachInterrupt(HALL, countHallPulse, FALLING);

  baro.init(0x76);
  myFile = SD.open("data.txt", FILE_WRITE);

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_BAUD_57600);
  delay(500);
  Serial1.end();
  
  delay(500);
  GPS.begin(57600);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);   // 1 Hz update rate

  for(uint32_t i = 0; i < 2000; i++)//let the baro initialize
  {
    baro.poll();
    delay(1);
  }
  startingAlt = baro.getAlt();
  
}

void loop() {  
  while(GPS.read());//parse all GPS chars available

  if (GPS.newNMEAreceived())
    GPS.parse(GPS.lastNMEA());

  baro.poll();
  currentAlt = baro.getAlt() - startingAlt;
  
  uint32_t curTime = millis();
  if(curTime < loopTime + 100)//if less than 100ms, start over
    return;
  
  loopTime = curTime;
  updateINA();
  updateSpeed();

  
  uint8_t btn = readBtn();
  updateThrottle(btn == 5);

  writeToBtSd();
}

void updateThrottle(uint8_t pressed)
{
  static int debounce = 0;
  if(pressed && debounce < 13)
    debounce++;

  if(!pressed && debounce > 0)
    debounce--;
  
  if(debounce > 10)//debounce thresh
  {
    float errorCurrent = TARGET_CURRENT - InaCurrent;
    throttle += errorCurrent * 0.012;

    if(errorCurrent < -4)//failsafe
      throttle = 0;
  }
  else
    throttle = 0;


  //Write over I2C
  uint16_t rawThrottle = throttle * 65535;
  Wire.beginTransmission(0x66);
  Wire.write(0x40);//throttle register
  Wire.write((rawThrottle >> 8) & 0xFF);
  Wire.write(rawThrottle & 0xFF);
  Wire.endTransmission();
}

uint8_t readBtn()
{
  uint16_t btnAnalog = analogRead(TEMP);
  //Serial.println(btnAnalog);

  uint8_t btn = 0;
  if(btnAnalog < 10)  btn = 1;
  if(btnAnalog < 320 && btnAnalog > 300)  btn = 2;
  if(btnAnalog < 145 && btnAnalog > 125)  btn = 3;
  if(btnAnalog < 495 && btnAnalog > 470)  btn = 4;
  if(btnAnalog < 735 && btnAnalog > 710)  btn = 5;
  
  return btn;
}

void updateINA()
{
  InaVoltage = INAvoltage();
  InaCurrent = INAcurrent();
  InaPower = InaVoltage * InaCurrent;
  
  double currentInaTime = millis();
  energyUsed += InaPower * (currentInaTime - lastInaMeasurement) / 1000;
  lastInaMeasurement = currentInaTime;  
}

void updateSpeed()
{
  currentSpeed = 1000000.0 / avgdT * WHEEL_CIRC; 
  if(micros() - lastHallPulse > 2000000)
    currentSpeed = 0;
  
  distance = distTicks * TICK_DIST;
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
  //uint32_t startMicros = micros();
  
  String outputStr = String(InaVoltage, 3) + " " + String(InaCurrent, 3) + " " + String(InaPower) + " "+ String(currentSpeed) + " " +
                     String(energyUsed) + " " + String(distance) + " " + String(currentAlt) + " " + 
                     String(batteryOK) + " "+ String(batteryVoltage) +" " + String(millis()) + " " + String(GPS.latitudeDegrees, 7) + 
                     " " + String(GPS.longitudeDegrees, 7) + " " + String(GPS.altitude) + " " + String(GPS.satellites);
  
  
  Serial.println(outputStr);//usb
  
  Serial2.println(outputStr);//bluetooth


  
  myFile.println(outputStr);
  myFile.flush();

  //Serial.println(micros() - startMicros);
}
