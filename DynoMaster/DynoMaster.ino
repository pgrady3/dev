#include <i2c_t3.h>

#define LED1 3
#define LED2 21

#define RELAY 2
#define HALL 23
#define WHEEL_TICKS 54

volatile uint32_t tickTimes[WHEEL_TICKS];
volatile uint32_t tickPos;

volatile uint32_t loopTicker = 0;
volatile uint32_t lastHallPulse = 0;
volatile int32_t avgdT = 1000000;
volatile uint32_t distTicks = 0;

uint32_t testBeginTime = 0;
uint32_t testActive = 0;

float InaVoltage = 0;
float InaCurrent = 0;
float InaPower = 0;
float currentRPM = 0;
float targetThrottle = 0;
float targetCurrent = 4;
float integralTerm = 0;

void setup() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  INAinit();

  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  pinMode(RELAY, OUTPUT);

  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  digitalWrite(RELAY, HIGH);

  pinMode(HALL, INPUT_PULLUP);

  analogWriteResolution(12);

  attachInterrupt(HALL, countHallPulse, FALLING);
}

void loop() {

  //Main loop delay-----------------------------------------
  uint32_t currentMillis = millis();
  if(currentMillis - loopTicker < 20)
    return;

  loopTicker = currentMillis;

  //Read sensors--------------------------------------------
  InaVoltage = INAvoltage();
  InaCurrent = INAcurrent();
  InaPower = InaVoltage * InaCurrent;
  currentRPM = 1000000.0 / avgdT * 60; 
  if(micros() - lastHallPulse > 2000000)
    currentRPM = 0;

  //Read serial port----------------------------------------
  while(Serial.available())
  {
    uint8_t c = Serial.read();

    if(c == 'b')
    {
      testBeginTime = currentMillis;
      testActive = 1;
      initTest();
    }
    
    if(c == 's')
      testActive = 0;

    if(c >= 'A' && c <='Z')//the jankiest shit ever. use letters as current targets
      targetCurrent = c - 'A' + 1;
  }

  //Begin state machine------------------------------------

  if(currentMillis - testBeginTime > 90000)
    testActive = 0;
    
  if(testActive)
    runTest(currentMillis - testBeginTime);
  else
    writeThrottle(0);
  
  //Print---------------------------------------------------
  Serial.print(InaVoltage);
  Serial.print(" ");
  Serial.print(InaCurrent);
  Serial.print(" ");
  Serial.print(InaPower);
  Serial.print(" ");
  Serial.print(currentRPM);
  Serial.print(" ");
  Serial.print(targetCurrent);
  Serial.print(" ");
  Serial.print(currentMillis);
  Serial.println();
}

void initTest()
{
  integralTerm = 0;
}

void runTest(uint32_t msElapsed)
{
  //Fault conditions
  if(    (msElapsed > 3000 && InaCurrent < 0.1) //no current after a long time
      || (InaCurrent > 20.0) //overcurrent
      || (InaVoltage < 12.0) //undervoltage
      || (currentRPM > 1000))
  {
    testActive = 0;
    writeThrottle(0);
    return;
  }

  float errorCurrent = targetCurrent - InaCurrent;
  integralTerm += errorCurrent * 0.0003;
  
  if(currentRPM < 100)    integralTerm = 0;

  float targetThrottle = 0.47 + currentRPM * 0.00065;//5A at 20v

  targetThrottle += integralTerm;
  
  //targetThrottle = 0.51 + currentRPM * 0.00068;//10A at 20v

  //Serial.println(integralTerm);
  writeThrottle(targetThrottle);
}

void writeThrottle(float pct)
{
  pct = constrain(pct, 0.0, 1.0);
  uint16_t dacVal = pct * 4095.0;
  analogWrite(A14, dacVal);

  digitalWrite(LED2, dacVal > 0);
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
  Wire.write(0b00001010);//64 averages, 1ms voltage sampling
  Wire.write(0b00000111);//1ms current sampling, free running
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
  uint32_t currentMicros = micros();

  tickTimes[tickPos++] = currentMicros;
  tickPos %= WHEEL_TICKS;

  avgdT = currentMicros - tickTimes[tickPos];

  distTicks++;
  
  lastHallPulse = currentMicros;

  digitalWrite(LED1, (distTicks) & 1);
}
