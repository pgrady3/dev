#define pin_pwm_pha INHA
#define pin_pwm_phb INHB
#define pin_pwm_phc INHC

void FOCinit(void);
void adc0_irq();
void dummyISR();
void FOCsetThrottle(int32_t t);
void Set_PWM_Duty_Scaled(long pwma, long pwmb, long pwmc);
void FOCFindOffset();

int FTM_Mod_Val;

#define MEM_SIZE 9000
#define START 100000

volatile int16_t phAmem[MEM_SIZE];
volatile int16_t phBmem[MEM_SIZE];
volatile uint16_t anglemem[MEM_SIZE];
uint32_t memPos = 0;
uint32_t ticks = 0;

int32_t throttle = 0;

//sets up ADC and timers
void FOCinit(void)
{
  // FTM0 settings
  // f_pwm = 20kHz
  // up-down counting mode
  // trigger out = FTM0_CH2 (channel unused for PWM generation)
  analogWriteRes(16);
  analogWriteFrequency(pin_pwm_pha, 20000*2); // Up-down mode screws with us, double the freq works
  FTM_Mod_Val = FTM0_MOD; // Make it easier on ourselves for later, just save this value                       
  FTM0_SC |= FTM_SC_CPWMS; // Turn on center aligned mode
  FTM0_EXTTRIG |= FTM_EXTTRIG_CH2TRIG; // Output trigger = Channel 2 compare flag (CH2 not used for PWM)
  FTM0_C2V = FTM_Mod_Val; // Channel 2 will fire at the top of the PWM up-down count,
                          // when PWM waveform is at its low level

  // ADC settings
  analogReadRes(12); // Set 12-bit ADC resolution (default is 10)
  ADC0_SC2 = ADC_SC2_ADTRG; // Hardware triggered (comes from PDB)
  ADC0_SC3 = 0; // Not continuous, no averaging
  ADC0_SC1A = 8; // Channel A set to measure A-phase current, channel 8, ARM pin 35, teensy pin 16
  ADC0_SC1B = 9 | ADC_SC1_AIEN; // Interrupt enabled for channel B, 
                                       // and it measures B-phase current, ARM pin 36, teensy pin 17
 
  // Programmable Delay Block settings
  // Clock it first! If the PDB clock isn't turned on, the processor crashes when
  // accessing PDB registers.
  SIM_SCGC6 |= SIM_SCGC6_PDB; // Enable PDB in System Integration Module
  
  PDB0_SC = PDB_SC_TRGSEL(8); // FTM0 trigger input selected (which was set to FTM0_CH2)
  PDB0_CH0DLY0 = 1; // Almost immediately trigger the first ADC conversion
  PDB0_CH0C1 = (2 << 16) | (1 << 8) | (3); // Back-to-back turned on for channel 2,
      // channel 1 set by its counter, and both channel 1 and 2 outputs turned on
      // Back-to-back mode means that channel 2 (ADC0 'B' conversion) will start
      // as soon as the channel 1 ('A' conversion) is completed.
  PDB0_MOD = FTM0_MOD; // Same maximum count as FTM
  PDB0_SC |= PDB_SC_LDOK | PDB_SC_PDBEN; // Turn on the PDB
  
  attachInterruptVector(IRQ_ADC0, adc0_irq); // When IRQ_ADC0 fires, code execution will
                                             // jump to "adc0_irq()" function.
  NVIC_ENABLE_IRQ(IRQ_ADC0); // ADC complete interrupt
  NVIC_SET_PRIORITY(IRQ_ADC0, 10); // Zero = highest priority

  
  analogWrite(pin_pwm_pha, 1);
  analogWrite(pin_pwm_phb, 1);
  analogWrite(pin_pwm_phc, 1);
  // Immediately change to 0% duty cycle
  FTM0_C1V = 0;
  FTM0_C0V = 0;
  FTM0_C6V = 0;
}

void FOCFindOffset()
{
  NVIC_DISABLE_IRQ(IRQ_ADC0); // ADC complete interrupt

  for(int i = 0;; i+= 20)
  {
      uint16_t angle = i;
    
      Park_Type pp;
      SVM_Type ss;
      
      pp.Ds = 6000;
      pp.Qs = 0000;
      pp.Theta = angle;
      inv_park_transform(&pp);
      ss.Alpha = pp.Alpha;
      ss.Beta = pp.Beta;
      svm_calc(&ss);
    
      Set_PWM_Duty_Scaled(ss.tA, ss.tB, ss.tC);

      if(i == 0)
        delay(1000);
        
      delay(1);

      ENCgetAbsAngle();
      
      Serial.print(angle);
      Serial.print(" ");
      Serial.print(ENCreadAngle());
      Serial.print(" ");
      Serial.println(ENCreadEAngle());
  }

  while(1)//dont return
    ;
}

void dummyISR()
{
  ticks++;
}

/* adc0_irq
 *  Interrupt vector for the ADC0 complete interrupt. In this program's setup, the
 *  ADC0 interrupt will only fire when the second conversion ('B' conversion) is 
 *  complete. The act of reading the result registers (ADC0_Rn) clears the conversion
 *  complete flag.
 */
void adc0_irq()
{
  // Clear the ADC interrupt flags by reading results  
  int16_t phA_current = ADC0_RA - 2054;
  int16_t phB_current = ADC0_RB - 2050;
  // FOC code can go here:

  if(memPos < MEM_SIZE && throttle == 12000)
  {
    anglemem[memPos] = ENCreadEAngle();
    phAmem[memPos] = phA_current;
    phBmem[memPos++] = phB_current;
  }

  ticks++;

  if((ticks & 0xF) == 0)
    ENCgetAbsAngle();
  
  if(ticks & 1)//The loop is called at 20kHz, so reduce calling rate to 10kHz
    return;
  
  digitalWrite(LED2, HIGH);
  
  Park_Type pp;
  SVM_Type ss;
  
  pp.Ds = 000;
  pp.Qs = throttle;
  pp.Theta = ENCreadEAngle();
  inv_park_transform(&pp);
  ss.Alpha = pp.Alpha;
  ss.Beta = pp.Beta;
  svm_calc(&ss);

  Set_PWM_Duty_Scaled(ss.tA, ss.tB, ss.tC);

  digitalWrite(LED2, LOW);
}

void FOCsetThrottle(int32_t t)
{
  throttle = t;
}

/* Set_PWM_Duty
 *  Inputs:
 *  pwma - Value to be loaded into the A-phase duty cycle register
 *  pwmb, pwmc - see pwma description
 *  
 *  Directly sets the duty cycle register (FTMn_CmV) with the input
 *  value.
 */
void Set_PWM_Duty(int pwma, int pwmb, int pwmc)
{
    FTM0_C1V = pwma;
    FTM0_C0V = pwmb;
    FTM0_C6V = pwmc;
}

/* Set_PWM_Duty_Scaled
 *  Inputs:
 *  pwma - integer representation of A-phase duty cycle (0->65535)
 *  pwmb, pwmc - see pwma description
 *  
 *  Scales the PWM command input to the active FTM_MOD register value.
 *  Allows the user to send a PWM duty cycle scaled to a 16-bit number
 */
void Set_PWM_Duty_Scaled(long pwma, long pwmb, long pwmc)
{
  Set_PWM_Duty((pwma*FTM_Mod_Val) >> 16,
               (pwmb*FTM_Mod_Val) >> 16,
               (pwmc*FTM_Mod_Val) >> 16);
}

