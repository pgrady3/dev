#define ANALOG_WRITE_RES 8

volatile uint8_t motorPos = 0;

void PWMSetMotorPos(uint8_t newPos)
{
  motorPos = newPos;
}

void PWMSetDuty(uint16_t duty)
{
  if(duty < 15)
    FTM0_SC &= ~FTM_SC_TOIE;//disable ISR since not reliable at low duty cycles
  else
    FTM0_SC |= FTM_SC_TOIE;

  if(duty > 250)
    duty = 260;//set so match will never occur, locking in a 100% pwm
  
  uint32_t cval = ((uint32_t)duty * (uint32_t)(FTM0_MOD + 1)) >> ANALOG_WRITE_RES;
  FTM0_C6V = cval;
}

void PWMInit()
{
  analogWriteFrequency(21, 30000);//set FTM0 to 20kHz
  analogWriteResolution(ANALOG_WRITE_RES);

  FTM0_C6SC = FTM_CSC_CHIE | FTM_CSC_MSA | FTM_CSC_ELSB;
  FTM0_SC |= FTM_SC_TOIE;//toie enable
  PWMSetDuty(0);
  NVIC_ENABLE_IRQ(IRQ_FTM0);
}

// write the phase to the low side gates
// 1-hot encoding for the phase
// 001 = A, 010 = B, 100 = C
void writeLow(uint8_t phase){
  digitalWriteFast(INLA, (phase&(1<<0)));
  digitalWriteFast(INLB, (phase&(1<<1)));
  digitalWriteFast(INLC, (phase&(1<<2)));
}

void writeHigh(uint8_t phase){
  digitalWriteFast(INHA, (phase&(1<<0)));
  digitalWriteFast(INHB, (phase&(1<<1)));
  digitalWriteFast(INHC, (phase&(1<<2)));
}

void writeLowState(uint8_t pos)
{
  writeHigh(0b000);
  switch(pos){
    case 0://LOW A, HIGH B
      writeLow(0b011);
      break;
    case 1://LOW A, HIGH C
      writeLow(0b101);
      break;
    case 2://LOW B, HIGH C
      writeLow(0b110);
      break;
    case 3://LOW B, HIGH A
      writeLow(0b011);
      break;
    case 4://LOW C, HIGH A
      writeLow(0b101);
      break;
    case 5://LOW C, HIGH B
      writeLow(0b110);
      break;
    default:
      writeLow(0);
      break;
  }
}

void writeHighState(uint8_t pos)
{
  switch(pos){
    case 0://LOW A, HIGH B
      writeLow(0b001);
      writeHigh(0b010);
      break;
    case 1://LOW A, HIGH C
      writeLow(0b001);
      writeHigh(0b100);
      break;
    case 2://LOW B, HIGH C
      writeLow(0b010);
      writeHigh(0b100);
      break;
    case 3://LOW B, HIGH A
      writeLow(0b010);
      writeHigh(0b001);
      break;
    case 4://LOW C, HIGH A
      writeLow(0b100);
      writeHigh(0b001);
      break;
    case 5://LOW C, HIGH B
      writeLow(0b100);
      writeHigh(0b010);
      break;
    default:
      writeLow(0);
      writeHigh(0);
      break;
  }
}

void ftm0_isr(void)
{
  if(FTM0_C6SC >> 7)//going DOWN
  {
    FTM0_C6SC = FTM0_C6SC & 0x7F;//clear flag
    writeLowState(motorPos);
  }
  else//going UP, start cycle
  {
    FTM0_SC = FTM0_SC & 0x7F;//clear flag
    writeHighState(motorPos);
  }
}
