void readH2();

void readH2()
{
  Wire.beginTransmission(H2_I2C_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(0x40, 1);

  if (Wire.available() < 1)
    return;

  uint8_t resp = Wire.read();
}
