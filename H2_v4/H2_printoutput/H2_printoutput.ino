#include <i2c_t3.h>
#include <Metro.h>
#include <stdio.h>
#include "INA.h"
#include "global.h"
#include "Comms.h"

#define LED1 10
#define LED2 11
#define LED3 13

#define SUPPLY_VALVE 5
#define PURGE_VALVE 6
#define FAN 4
#define FAN_READ 17
#define THERM 21

#define BUZZER 9
#define SHORT_CIRCUIT 8
#define PASS 3

int incomingByte = 0;

uint32_t printInt = 50;

int short_start = 999999999;
int start_Purge_delay = 999999999;
int start_Purge = 999999999;

int setpoint = 16.95;

float flow = 0;
float total = 0;
float h2energy = 0;
float eff = 0;
float flowcurrent = 0;
float leak = 0;
int purgeCount = 1;

#define FLOWMETER_BUF_SIZE 100
char flowmeterBuf[FLOWMETER_BUF_SIZE];
uint32_t flowmeterBufPos = 0;

const uint32_t Short_StartupIntervals[6] = {0, 616, 616, 616, 313, 313};
const uint32_t Short_StartupDurations[6] = {50, 50, 50, 50, 100, 50};
uint8_t Short_StartupIndex = 0;

uint32_t Short_Intervals[1] = {100};
const uint32_t Short_Durations[1] = {20};
uint8_t Short_ind = 0;
uint32_t Purge_Intervals[1] = {300};
const uint32_t Purge_Durations[1] = {150};
uint8_t Purge_ind = 0;

uint32_t Short_Interval = Short_Intervals[0]*1000;
uint32_t Short_Duration = Short_Durations[0];
uint32_t Purge_Interval = Purge_Intervals[0]*1000;
uint32_t Purge_Duration = Purge_Durations[0];
Metro Short_IntervalTimer = Metro(Short_Interval);
Metro Short_DurationTimer = Metro(Short_Duration);
Metro Purge_IntervalTimer = Metro(Purge_Interval);
Metro Purge_DurationTimer = Metro(Purge_Duration);

void setup() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Comms_setup();
  Serial.begin(115200);
  Serial1.begin(57600);
  UART0_C3 = 16;//tx invert
  UART0_S2 = 16;//rx invert

  analogReadResolution(12);
  
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
  pinMode(SUPPLY_VALVE, OUTPUT);
  digitalWrite(SUPPLY_VALVE, HIGH);
  pinMode(PURGE_VALVE, OUTPUT);
  digitalWrite(PURGE_VALVE, LOW);
  pinMode(FAN, OUTPUT);
  analogWriteFrequency(FAN,100000);
  pinMode(FAN_READ, INPUT);
  pinMode(THERM, INPUT);

  bootup();
  
}

void loop() {

  uint32_t cur = millis();
  static uint32_t lastTime = 0;
  if(cur - lastTime >= printInt)
  {
    lastTime = cur;
    getData();
    printData(cur);
    readInputs();
  }
  
  updateShort();
  updatePurge();
}

void bootup(){
  digitalWrite(FAN,LOW);
  
  purge(3000);

  for(uint8_t ind = 0; ind<(sizeof(Short_StartupIntervals)/sizeof(uint32_t)); ind++){
    delay(Short_StartupIntervals[ind]);
    FCShort(Short_StartupDurations[ind]);
  }

  analogWrite(FAN,HIGH);
  delay(100);
  analogWrite(FAN,fanSpeed*255);

  short_start = millis();
}

void purge(int duration)
{
  digitalWrite(PURGE_VALVE,HIGH);
  delay(duration);
  digitalWrite(PURGE_VALVE,LOW);
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

void getData(){
  
  static uint32_t lastIntegrationMicros = micros();
  uint32_t curMicros = micros();
  
  // Fuel cell numbers
  voltage = INAvoltage();
  current = INAcurrent();
  power = voltage*current;
  totFCNRG = totFCNRG + (power*(curMicros - lastIntegrationMicros))/1e6;
  health = current - (voltage - setpoint)/(-0.75);  
  lastIntegrationMicros = curMicros;
  
  // fan
  fan_current = analogRead(FAN_READ) / 4096.0 * 3.3 / 0.20;

  // thermistor reading
  prct = analogRead(THERM)/4096.0;
  temp = ((20 + (prct/(1-prct)*1208 - 1076)/3.8) * 9/5 + 32);
  
  // flowmeter reading
//  mass = massFlow;
  readFlowmeter();
  flow = massFlow[0];  // mg/s
  total = massFlow[1]; // g
  h2energy = flow/1000*119.96e3;     // W
  totH2NRG = total*119.96e3;
  
  // efficiency and leaks
  eff = power/h2energy*100;
  flowcurrent = flow/2.01588 * 2 * 6.0221409e23 * 1.60217733e-19 / 20; // A
  leak = (flowcurrent - current)/flowcurrent*100;

  allEff = totFCNRG/totH2NRG*100;
  
}

void readFlowmeter()
{
  Serial1.print("A\r");
  while(Serial1.available())
  {
    flowmeterBuf[flowmeterBufPos++] = Serial1.read();
    if(flowmeterBufPos >= FLOWMETER_BUF_SIZE)
      flowmeterBufPos = FLOWMETER_BUF_SIZE - 1;
    if(flowmeterBuf[flowmeterBufPos - 1] == '\r')
      break;
  }
  
  flowmeterBuf[flowmeterBufPos] = 0;
  
//  Serial.println(flowmeterBuf);
  char* buf;
//  float massFlow[2] = {0,0};
  int pos = 0;
  buf = strtok (flowmeterBuf," ");
  
  while (buf != NULL)
  {
    if(pos == 1)
      flowPres = atof(buf);
    if(pos == 4)
      massFlow[0] = atof(buf);
    if(pos == 5)
      massFlow[1] = atof(buf);
    pos++;
    buf = strtok (NULL, " ");
  }
  flowmeterBufPos = 0;
    
//  return massFlow;
}

void printData(uint32_t cur){

  Serial.print(flow,4);
  Serial.print(" ");
  Serial.print(power,4);
  Serial.print(" ");
  Serial.print(eff,4);
  Serial.print(" ");
  Serial.print(health,4);
  Serial.print(" ");
  Serial.print(voltage,4);
  Serial.print(" ");
  Serial.print(current,4);
  Serial.print(" ");
  Serial.print(leak,4);
  Serial.print(" ");
  Serial.print(cur);
  Serial.print(" ");
  Serial.print(total,4);
  Serial.print(" ");
  Serial.print(allEff,4);
  Serial.print(" ");
  Serial.print(totFCNRG,4);
  Serial.print(" ");
  Serial.println(flowPres,4);
}

void readInputs(){
  if (Serial.available() > 0) {
        incomingByte = Serial.read();
        
        if(incomingByte=='p'){
          FCPurge_Start();
        }
        if(incomingByte=='s'){
          FCShort_Start();
          purgeCount++;
        }
        if(incomingByte >= '0' && incomingByte <= '9'){
          float val = (incomingByte - '0') / 10.0;
          changeFan(val);                
        }
        
   }
}

void changeFan(double fanSpeed){
  analogWrite(FAN,HIGH);
  delay(100);
  analogWrite(FAN,fanSpeed*255);
}

void updateShort(){
// health based purge
//  if(health < 0 && (millis() - short_start) > 10000 && voltage > 13.5 && voltage < 17){

// timed short/purge
   if(abs((millis() - short_start) - 100000) < 5){
    FCShort_Start();
    purgeCount++;
   }
   
   if(millis() - short_start >= Short_Duration){
    FCShort_Stop();
   }
}

void updatePurge()
{
  if(purgeCount % 6 == 0){
      start_Purge_delay = millis();
  }
  
  if(abs(millis() - start_Purge_delay - 5000) < 5){
    FCPurge_Start();
   }

   if(millis() - start_Purge >= Purge_Duration){
    FCPurge_Stop();
   }
}

void FCShort_Start()
{
  
  short_start = millis();
  digitalWrite(PASS, LOW);
  delay(1);
  digitalWrite(SHORT_CIRCUIT, HIGH);
}

void FCShort_Stop()
{
  
  digitalWrite(SHORT_CIRCUIT, LOW);
  delay(2);
  digitalWrite(PASS, HIGH);
}

void FCPurge_Start()
{
  start_Purge_delay = 999999999;
  start_Purge = millis();
  digitalWrite(PURGE_VALVE,HIGH);
}

void FCPurge_Stop()
{
  digitalWrite(PURGE_VALVE,LOW);
}





