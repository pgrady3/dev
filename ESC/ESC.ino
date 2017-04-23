#include "TimerOne.h"
#include "arm_math.h"
#include "SPI.h"
#include "config.h"
#include "hall.h"
#include "fixed_foc.h"
#include "enc.h"
#include "foc.h"

float actualThrottle = 0;

void setup(){
  init();
  ENCinit();
  FOCinit();

  pinMode(THROTTLE, INPUT);
  
  //FOCFindOffset();
  //ENCFindVar();
  //FOCsetThrottle(15000);
  delay(3000);
}

void loop(){

  float tempThrottle = getThrottle() * 60000;

  if(tempThrottle > actualThrottle)
    actualThrottle += 1000;
  else
    actualThrottle = tempThrottle;
    
  FOCsetThrottle(actualThrottle);
  delay(30);

  int32_t temp = avgDAngle;
  //Serial.println(temp);
  
  /*if(memPos == MEM_SIZE)
  {
    FOCsetThrottle(0);
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
  }*/
}


