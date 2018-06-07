#include "ms5611.h"

void MS5611::poll(void)
{
  uint32_t D1 = 0;
  uint32_t D2 = 0;

  
  if(convertTemp)                   //Currently doing a temp conversion
  {
    if(millis() < convertTime + 8)  //our conversion hasn't finished yet
      return;

    convertTime = millis();
    D2 = getConv();

    if(avgD2 == 0)
      avgD2 = D2;
    else
      avgD2 += ((int64_t)D2 - avgD2) / 16;
    
    Wire.beginTransmission(addr);
    Wire.write(0x48);               //Start D1 conversion, OSR=4096
    Wire.endTransmission();

    convertTemp = 0;
  }
  else                              //Doing a pressure conversion
  {
    if(millis() < convertTime + 12)  //our conversion hasn't finished yet
      return;

    convertTime = millis();
    D1 = getConv();

    if(avgD1 == 0)
      avgD1 = D1;
    else
      avgD1 += ((int64_t)D1 - avgD1) / 16;
    
    Wire.beginTransmission(addr);
    Wire.write(0x56);               //Start D2 conversion, OSR=2048
    Wire.endTransmission();

    compute(avgD1, avgD2);
    /*Serial.print(alt);
    Serial.print(" ");
    BaroUpdate(avgD1, secondAvgD2);
    Serial.println(alt);*/
    
    convertTemp = 1;
  }
}

void MS5611::compute(uint32_t D1, uint32_t D2)
{
  int32_t dT = D2 - (prom[5] << 8);
  int32_t Temp = (dT * prom[6]) >> 23;
  Temp += 2000;
  
  int64_t Off = ((int64_t)prom[2]) << 17;
  Off += (prom[4] * (int64_t)dT) / 64;
  int64_t Sens = (prom[1] << 16) + (prom[3] * (int64_t)dT) / 128;
  float P = ((D1 * Sens) / 2097152 - Off) / 32768.0;

  if(P < 100)
    return;
  
  alt = 44330 * (1 - pow((float)P / 101325.0f, 1.0f/5.255f));
  altHistory[altHistoryPos++] = alt;
  altHistoryPos %= MS5611_ALT_HISTORY_LEN;

  for(uint8_t i = 0; i < MS5611_ALT_HISTORY_LEN; i++)
    avgAlt += altHistory[i];

  avgAlt /= MS5611_ALT_HISTORY_LEN;
}

uint32_t MS5611::getConv(void)
{
  Wire.beginTransmission(addr);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(addr, (uint8_t)3);
  uint32_t ans = 0;
  ans += (uint32_t)Wire.read() << 16;
  ans += (uint32_t)Wire.read() << 8;
  ans += (uint32_t)Wire.read();
  return ans;
}

void MS5611::init(uint8_t _addr)
{
  addr = _addr;
  
  for(uint8_t i = 0; i < MS5611_ALT_HISTORY_LEN; i++)
    altHistory[i] = 0;
  
  //Wire.begin();
  reset();

  for(uint8_t i = 0; i < 7; i++)
  {
    prom[i] = getProm(i);
  }

  
  /*Serial.println("MS5637 PROM values");
  for(uint8_t i = 0; i < 7; i++)
  {
    Serial.print(i);
    Serial.print(':');
    Serial.println(prom[i], HEX);
  }*/
}

void MS5611::reset(void)
{
  Wire.beginTransmission(addr);
  Wire.write(0x1E);
  Wire.endTransmission();
  delay(100);
}

uint32_t MS5611::getProm(uint8_t promAddr)
{
  promAddr %= 7;
  promAddr = 0xA0 | (promAddr << 1);

  Wire.beginTransmission(addr);
  Wire.write(promAddr);
  Wire.endTransmission();

  Wire.requestFrom(addr, (uint8_t)2);
  uint32_t ans = 0;
  ans += ((uint16_t)Wire.read()) << 8;
  ans += Wire.read();
  return ans;
}

float MS5611::getAlt(void)
{
  return avgAlt;
}
