#include <i2c_t3.h>

#define H2_I2C_ADDR 0x60
#define I2C_READ_FCV  0x10
#define I2C_READ_FCI 0x11
#define I2C_READ_FCE 0x12
#define I2C_READ_FCTEMP 0x13
#define I2C_READ_H2PRESS 0x14
#define I2C_READ_H2FLOW  0x15
#define I2C_READ_H2TOT 0x16

#define I2C_WRITE_PURGE 0x50
#define I2C_WRITE_SHORT 0x51
#define I2C_WRITE_CLOSESUPPLY 0x52
#define I2C_WRITE_OPENSUPPLY 0x53
#define I2C_WRITE_SETFAN 0x54

uint8_t lastCmd = 0;
uint8_t i2cmem[4];

void i2cmemStore(int32_t value){
  i2cmem[0] = value & 0xFF;
  i2cmem[1] = (value >> 8) & 0xFF;
  i2cmem[2] = (value >> 16) & 0xFF;
  i2cmem[3] = (value >> 24) & 0xFF;
}

void i2cRequestEvent(void){
  Wire1.write(&i2cmem[0],4);  
}

void i2cReceiveEvent(size_t count) {
  if(count){
    lastCmd = Wire1.readByte();
    
    if(lastCmd == I2C_READ_FCV)
      i2cmemStore((int32_t) (voltage*1000));
        
    if(lastCmd == I2C_READ_FCI)
      i2cmemStore((int32_t) (current*1000));

    if(lastCmd == I2C_READ_FCE)
      i2cmemStore((int32_t) (totFCNRG*1000));

    if(lastCmd == I2C_READ_FCTEMP)
      i2cmemStore((int32_t) (temp*1000));

    if(lastCmd == I2C_READ_H2PRESS)
      i2cmemStore((int32_t) (flowPres*1000));
    
    if(lastCmd == I2C_READ_H2FLOW)
      i2cmemStore((int32_t) (massFlow[0]*1000));

    if(lastCmd == I2C_READ_H2TOT)
      i2cmemStore((int32_t) (massFlow[1]*10000));//10 thousand

    if(lastCmd == I2C_WRITE_OPENSUPPLY)
      digitalWrite(SUPPLY_VALVE, HIGH);

    if(lastCmd == I2C_WRITE_CLOSESUPPLY)
      digitalWrite(SUPPLY_VALVE, LOW);
      
    if(lastCmd == I2C_WRITE_SHORT)
      FCShort_Start();
      
    if(lastCmd == I2C_WRITE_PURGE)
      FCPurge_Start();
  }
}

void Comms_setup(){
  Wire1.begin(H2_I2C_ADDR);
  Wire1.setDefaultTimeout(100);
  
  Wire1.onReceive(i2cReceiveEvent);
  Wire1.onRequest(i2cRequestEvent);

}

