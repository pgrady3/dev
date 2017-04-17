#include "TimerOne.h"
#include "arm_math.h"
#include "SPI.h"
#include "config.h"
#include "hall.h"
#include "enc.h"
#include "fixed_foc.h"
#include "foc.h"

void setup(){
  init();
  ENCinit();
  FOCinit();

  delay(3000);

  //FOCFindOffset();
  FOCsetThrottle(7000);
  delay(3000);
}

void loop(){
  FOCsetThrottle(12000);
  delay(1);

  if(memPos == MEM_SIZE)
  {
    //FOCsetThrottle(0);
    delay(3000);
    for(uint32_t i = 0; i < MEM_SIZE; i++)
    {
      Serial.print(anglemem[i]);
      Serial.print(" ");
      Serial.print(phAmem[i]);
      Serial.print(" ");
      Serial.println(phBmem[i]);
      delay(1);
    }

    while(1)
      ;
  }
}

