#ifndef MS5611_h
#define MS5611_h

#include "Arduino.h"
#include <i2c_t3.h>
//#include <Wire.h>

#define MS5611_ALT_HISTORY_LEN     16
#define MS5611_SAMPLES_SEC   52

class MS5611
{
  public:
    void init(uint8_t _addr);
    void poll();
    float getAlt(void);
  private:
    uint32_t  getProm(uint8_t promAddr);
    void reset(void);
    uint32_t getConv(void);
    void compute(uint32_t D1, uint32_t D2);

    uint8_t addr;
    uint32_t prom[7];
    uint8_t convertTemp = 0;
    uint32_t convertTime = 0;
    uint32_t avgD2 = 0;
    uint32_t avgD1 = 0;
    
    float alt = 0;
    float altHistory[MS5611_ALT_HISTORY_LEN];
    uint8_t altHistoryPos = 0;
    float avgAlt = 0;
};

#endif
