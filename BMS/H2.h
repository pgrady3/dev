#define H2_I2C_ADDR 0x60
#define I2C_READ_FCV  0x10
#define I2C_READ_FCI 0x11
#define I2C_READ_FCE 0x12
#define I2C_READ_FCTEMP 0x13
#define I2C_READ_H2PRESS 0x14
#define I2C_READ_H2FLOW  0x15
#define I2C_READ_H2TOT 0x16
#define I2C_READ_H2AVGEFF 0x17

#define I2C_WRITE_PURGE 0x50
#define I2C_WRITE_SHORT 0x51
#define I2C_WRITE_CLOSESUPPLY 0x52
#define I2C_WRITE_OPENSUPPLY 0x53
#define I2C_WRITE_SETFAN 0x54

#define I2C_WRITE_REPORTCURRENT 0x55
#define I2C_WRITE_LOADSHORT 0x56
#define I2C_WRITE_TIMESHORT 0x57



int32_t readH2(uint8_t reg);
void writeH2(uint8_t reg, int32_t val);

double mgtoJ(double mg)//convert milligrams of H2 to joules
{
  return mg * 119.93;
}

int32_t readH2(uint8_t reg)
{
  Wire.beginTransmission(H2_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(H2_I2C_ADDR, 4);

  uint32_t resp = 0;
  uint32_t count = 0;
  while(Wire.available())
  {
    count++;
    uint32_t c = Wire.read();
    resp = (resp >> 8) | (c << 24);
  }

  if(count == 4)
    return (int32_t)resp;

  return 0;
}

void writeH2(uint8_t reg, int32_t val)
{
  Wire.beginTransmission(H2_I2C_ADDR);
  Wire.write(reg);

  uint32_t uval = (uint32_t)val;
  Wire.write(uval & 0xFF);
  Wire.write(uval >> 8);
  Wire.write(uval >> 16);
  Wire.write(uval >> 24);

  Wire.endTransmission();
}
