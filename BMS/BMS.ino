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

#define TARGET_CURRENT 5

volatile uint32_t tickTimes[WHEEL_TICKS];
volatile uint32_t tickPos;

volatile uint32_t loopTime = 0;
volatile uint32_t lastHallPulse = 0;
volatile uint32_t lastInaMeasurement = 0;
volatile uint32_t countIntervals = 0;
volatile int32_t avgdT = 1000000;
volatile uint32_t distTicks = 0;

uint32_t shortTime = 0;

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
double FCV = 0;
double FCI = 0;
double FCE = 0;
double FCTemp = 0;
double H2Press = 0;
double H2Flow = 0;
double H2Tot = 0;
double H2Eff = 0;
double H2AvgEff = 0;

bool batteryOK = true;
uint8_t powerSaveVote = 0;
uint32_t h2Detected = 0;

File myFile;
Adafruit_GPS GPS(&Serial1);
//MS5611 baro;

void setup() {
  setupWatchdog();
  
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  //Wire.setDefaultTimeout(100);//this makes i2c not work?
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
  digitalWrite(RELAY, LOW);
  digitalWrite(SOLENOID, HIGH);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  pinMode(HALL, INPUT_PULLUP);
  attachInterrupt(HALL, countHallPulse, FALLING);

  myFile = SD.open("data.txt", FILE_WRITE);

  GPSInit();

  kickDog();
}


void loop() {  
  GPSPoll();//must be called rapidly
  
  uint32_t curTime = millis();
  if(curTime < loopTime + 100)//if less than 100ms, start over
    return;

  kickDog();//reset watchdog. sometimes i2c causes the processor to hang
  digitalWrite(LED2, !digitalRead(LED2));
  loopTime = curTime;

  updateINA();
  updateSpeed();
  pollH2();
  
  uint8_t btn = readBtn();
  updateThrottle(btn == 5);
  updateH2Btn(btn);

  writeToBtSd();

  if(InaCurrent > 20 || powerSaveVote == 0)
  {
    digitalWrite(RELAY, LOW);
    shortTime = curTime;
  }
  else if(curTime - shortTime > 2000 && powerSaveVote > 0)
    digitalWrite(RELAY, HIGH);
}

void pollH2()
{
  FCV = readH2(I2C_READ_FCV) / 1000.0;
  FCI = readH2(I2C_READ_FCI) / 1000.0;
  FCE = readH2(I2C_READ_FCE) / 1000.0;
  FCTemp = readH2(I2C_READ_FCTEMP) / 1000.0;
  H2Press = readH2(I2C_READ_H2PRESS) / 1000.0;
  H2Flow = readH2(I2C_READ_H2FLOW) / 1000.0;
  H2Tot = readH2(I2C_READ_H2TOT) / 10000.0;//10 thousand
  H2AvgEff = readH2(I2C_READ_H2AVGEFF) / 1000.0;
  
  H2Eff = FCV * FCI / mgtoJ(H2Flow);
}

void updateThrottle(uint8_t pressed)
{
  static int debounce = 0;
  if(pressed && debounce < 40)
    debounce++;

  if(!pressed && debounce > 0)
    debounce -= 5;
  
  if(debounce > 5)//debounce relay thresh
    powerSaveVote = 1;
  else
    powerSaveVote = 0;//if this is 0, the relay will turn off when not using cruise control

  //Serial.println(debounce);
  
  if(debounce > 30)//debounce throttle thresh
  {
    float errorCurrent = TARGET_CURRENT - InaCurrent;
    throttle += errorCurrent * 0.006;

    if(throttle > 1)
      throttle = 1;
    
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

void updateH2Btn(uint8_t btn)
{
  static uint32_t btnDuration = 0;
  static uint32_t btnSelected = 0;

  if(btn != btnSelected)
  {
    btnSelected = btn;
    btnDuration = 0; 
  }
  else
    btnDuration++;

  if(btnSelected == 1 && btnDuration == 5)
    writeH2(I2C_WRITE_PURGE, 0);

  if(btnSelected == 2 && btnDuration == 5)
    writeH2(I2C_WRITE_SHORT, 0);

  if(btnSelected == 3 && btnDuration == 5)
    writeH2(I2C_WRITE_LOADSHORT, 0);

  if(btnSelected == 4 && btnDuration == 5)
    writeH2(I2C_WRITE_TIMESHORT, 0);


  float clampedCurrent = InaCurrent * 10.0;

  if(clampedCurrent > 200)
    clampedCurrent = 200;

  if(clampedCurrent < 0)
    clampedCurrent = 0;
    
  writeH2(I2C_WRITE_REPORTCURRENT, (uint8_t)clampedCurrent);
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

  uint32_t prevTime = tickTimes[tickPos];// time 1 rev ago. fixed 7/5/2018 :(

  tickTimes[tickPos++] = current;
  tickPos %= WHEEL_TICKS;

  avgdT = current - prevTime;

  distTicks++;
  
  lastHallPulse = current;

  digitalWrite(LED1, (distTicks) & 1);
}

void GPSInit()
{
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_BAUD_57600);
  delay(500);
  Serial1.end();
  
  delay(500);
  GPS.begin(57600);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);   // 10 Hz update rate
}

void GPSPoll()
{
  while(GPS.read());
  
  if (GPS.newNMEAreceived())
    GPS.parse(GPS.lastNMEA());
}

void writeToBtSd() {
  //uint32_t startMicros = micros();
  
  String outputStr = String(InaVoltage, 3) + " " + String(InaCurrent, 3) + " " + String(InaPower) + " "+ String(currentSpeed) + " " +
                     String(energyUsed) + " " + String(distance) + " " + String(FCV, 3) + " " + 
                     String(FCI, 3) + " "+ String(FCE, 1) +" " + String(millis()) + " " + String(GPS.latitudeDegrees, 7) + 
                     " " + String(GPS.longitudeDegrees, 7) + " " + String(FCTemp, 1) + " " + String(H2Press, 1) + " " + String(H2Flow, 3) + " " + String(H2Tot, 4) + " " + String(H2Eff, 4) + " " + String(H2AvgEff, 4);
  
  
  Serial.println(outputStr);//usb  
  GPSPoll();//super hacky bc short GPS buffer
  Serial2.println(outputStr);//bluetooth
  GPSPoll();
  myFile.println(outputStr);
  myFile.flush();

  //Serial.println(micros() - startMicros);
}


void kickDog()
{
  noInterrupts();
  WDOG_REFRESH = 0xA602;
  WDOG_REFRESH = 0xB480;
  interrupts();
}

void setupWatchdog()
{
  kickDog();
  
  noInterrupts();                                         // don't allow interrupts while setting up WDOG
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;                         // unlock access to WDOG registers
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
  delayMicroseconds(1);                                   // Need to wait a bit..
  
  // about 2-3 second timeout
  WDOG_TOVALH = 0x0550;
  WDOG_TOVALL = 0x0000;
  
  // This sets prescale clock so that the watchdog timer ticks at 7.2MHz
  WDOG_PRESC  = 0x400;
  
  // Set options to enable WDT. You must always do this as a SINGLE write to WDOG_CTRLH
  WDOG_STCTRLH |= WDOG_STCTRLH_ALLOWUPDATE |
      WDOG_STCTRLH_WDOGEN | WDOG_STCTRLH_WAITEN |
      WDOG_STCTRLH_STOPEN | WDOG_STCTRLH_CLKSRC;
  interrupts();

  kickDog();
}

