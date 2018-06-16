#include <i2c_t3.h>
#include <Metro.h>
#include "INA.h"

#define LED1 10
#define LED2 11
#define LED3 13

#define SUPPLY_VALVE 5
#define PURGE_VALVE 6
#define FAN 4

#define BUZZER 9
#define SHORT_CIRCUIT 8
#define PASS 3
int incomingByte = 0;
double fanSpeed = .4;
double voltage = 0;
double current = 0;
double power = 0;
char flow;

const uint32_t Short_StartupIntervals[6] = {0, 616, 616, 616, 313, 313};
const uint32_t Short_StartupDurations[6] = {50, 50, 50, 50, 100, 50};
uint8_t Short_StartupIndex = 0;

uint32_t Short_Intervals[1] = {100};
const uint32_t Short_Durations[1] = {20};
uint8_t Short_ind = 0;
uint32_t Purge_Intervals[1] = {300};
const uint32_t Purge_Durations[1] = {175};
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
  Serial.begin(115200);
  Serial1.begin(57600);
  UART0_C3 = 16;//tx invert
  UART0_S2 = 16;//rx invert


  
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


  bootup();
  
}

void loop() {
  digitalWrite(LED2, digitalRead(LED1));
  digitalWrite(LED1, !digitalRead(LED1));
  voltage = INAvoltage();
  current = INAcurrent();
  power = voltage*current;
  
  Serial.print(voltage, 4);
  Serial.print(" ");
  Serial.print(current, 4);
  Serial.print(" ");
  Serial.println(power, 4);
  delay(200);
  
  Serial1.print("A\r");
  while(Serial1.available())
  {
    Serial.print((char)Serial1.read());
  }
//  Serial.println(flow(1));
  Serial.println();

  if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();

                // say what you got:
                if(incomingByte=='p'){
                  purge(1000);
                  }
                if(incomingByte=='s'){
                  FCShort(20);
                  delay(1000);
                }
//                Serial.print("I received: ");
//                Serial.println(incomingByte, DEC);
                // p = 112, s = 115
   }

   if(voltage > 13.5 && current < (voltage - 16.5)/(-0.75)){
    FCShort(20);
    delay(3000);
   }
  
}

void bootup(){
  Serial.println("Booting Up");
  digitalWrite(FAN,LOW);
  
  purge(3000);

  for(uint8_t ind = 0; ind<(sizeof(Short_StartupIntervals)/sizeof(uint32_t)); ind++){
    delay(Short_StartupIntervals[ind]);
    FCShort(Short_StartupDurations[ind]);
  }

  analogWrite(FAN,HIGH);
  delay(100);
  analogWrite(FAN,fanSpeed*255);
  
  
}

void purge(int duration)
{
  Serial.println("Purging");
  digitalWrite(PURGE_VALVE,HIGH);
  delay(duration);
  digitalWrite(PURGE_VALVE,LOW);
}

void FCShort(uint32_t duration)
{
  Serial.println("Shorting");
  digitalWrite(PASS, LOW);
  delay(1);
  digitalWrite(SHORT_CIRCUIT, HIGH);
  delay(duration);
  digitalWrite(SHORT_CIRCUIT, LOW);
  delay(2);
  digitalWrite(PASS, HIGH);
}




