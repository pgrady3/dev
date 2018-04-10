#define H2_I2C_ADDR 0x60

int32_t readH2(uint8_t reg);
void writeH2(uint8_t reg, int32_t val);


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
