#include "TimerOne.h"
#include "arm_math.h"
#include "SPI.h"
#include "config.h"
#include "hall.h"
#include "enc.h"

#define pin_pwm_pha INHA
#define pin_pwm_phb INHB
#define pin_pwm_phc INHC

int FTM_Mod_Val;

float dummy = 3;

const float sinApprox[] = 
{0.000f, 0.017f, 0.035f, 0.052f, 0.070f, 0.087f, 0.105f, 0.122f, 0.139f, 0.156f, 
0.174f, 0.191f, 0.208f, 0.225f, 0.242f, 0.259f, 0.276f, 0.292f, 0.309f, 0.326f, 
0.342f, 0.358f, 0.375f, 0.391f, 0.407f, 0.423f, 0.438f, 0.454f, 0.469f, 0.485f, 
0.500f, 0.515f, 0.530f, 0.545f, 0.559f, 0.574f, 0.588f, 0.602f, 0.616f, 0.629f, 
0.643f, 0.656f, 0.669f, 0.682f, 0.695f, 0.707f, 0.719f, 0.731f, 0.743f, 0.755f, 
0.766f, 0.777f, 0.788f, 0.799f, 0.809f, 0.819f, 0.829f, 0.839f, 0.848f, 0.857f, 
0.866f, 0.875f, 0.883f, 0.891f, 0.899f, 0.906f, 0.914f, 0.921f, 0.927f, 0.934f, 
0.940f, 0.946f, 0.951f, 0.956f, 0.961f, 0.966f, 0.970f, 0.974f, 0.978f, 0.982f, 
0.985f, 0.988f, 0.990f, 0.993f, 0.995f, 0.996f, 0.998f, 0.999f, 0.999f, 1.000f, 
1.000f, 1.000f, 0.999f, 0.999f, 0.998f, 0.996f, 0.995f, 0.993f, 0.990f, 0.988f, 
0.985f, 0.982f, 0.978f, 0.974f, 0.970f, 0.966f, 0.961f, 0.956f, 0.951f, 0.946f, 
0.940f, 0.934f, 0.927f, 0.921f, 0.914f, 0.906f, 0.899f, 0.891f, 0.883f, 0.875f, 
0.866f, 0.857f, 0.848f, 0.839f, 0.829f, 0.819f, 0.809f, 0.799f, 0.788f, 0.777f, 
0.766f, 0.755f, 0.743f, 0.731f, 0.719f, 0.707f, 0.695f, 0.682f, 0.669f, 0.656f, 
0.643f, 0.629f, 0.616f, 0.602f, 0.588f, 0.574f, 0.559f, 0.545f, 0.530f, 0.515f, 
0.500f, 0.485f, 0.469f, 0.454f, 0.438f, 0.423f, 0.407f, 0.391f, 0.375f, 0.358f, 
0.342f, 0.326f, 0.309f, 0.292f, 0.276f, 0.259f, 0.242f, 0.225f, 0.208f, 0.191f, 
0.174f, 0.156f, 0.139f, 0.122f, 0.105f, 0.087f, 0.070f, 0.052f, 0.035f, 0.017f, 
0.000f, -0.017f, -0.035f, -0.052f, -0.070f, -0.087f, -0.105f, -0.122f, -0.139f, -0.156f, 
-0.174f, -0.191f, -0.208f, -0.225f, -0.242f, -0.259f, -0.276f, -0.292f, -0.309f, -0.326f, 
-0.342f, -0.358f, -0.375f, -0.391f, -0.407f, -0.423f, -0.438f, -0.454f, -0.469f, -0.485f, 
-0.500f, -0.515f, -0.530f, -0.545f, -0.559f, -0.574f, -0.588f, -0.602f, -0.616f, -0.629f, 
-0.643f, -0.656f, -0.669f, -0.682f, -0.695f, -0.707f, -0.719f, -0.731f, -0.743f, -0.755f, 
-0.766f, -0.777f, -0.788f, -0.799f, -0.809f, -0.819f, -0.829f, -0.839f, -0.848f, -0.857f, 
-0.866f, -0.875f, -0.883f, -0.891f, -0.899f, -0.906f, -0.914f, -0.921f, -0.927f, -0.934f, 
-0.940f, -0.946f, -0.951f, -0.956f, -0.961f, -0.966f, -0.970f, -0.974f, -0.978f, -0.982f, 
-0.985f, -0.988f, -0.990f, -0.993f, -0.995f, -0.996f, -0.998f, -0.999f, -0.999f, -1.000f, 
-1.000f, -1.000f, -0.999f, -0.999f, -0.998f, -0.996f, -0.995f, -0.993f, -0.990f, -0.988f, 
-0.985f, -0.982f, -0.978f, -0.974f, -0.970f, -0.966f, -0.961f, -0.956f, -0.951f, -0.946f, 
-0.940f, -0.934f, -0.927f, -0.921f, -0.914f, -0.906f, -0.899f, -0.891f, -0.883f, -0.875f, 
-0.866f, -0.857f, -0.848f, -0.839f, -0.829f, -0.819f, -0.809f, -0.799f, -0.788f, -0.777f, 
-0.766f, -0.755f, -0.743f, -0.731f, -0.719f, -0.707f, -0.695f, -0.682f, -0.669f, -0.656f, 
-0.643f, -0.629f, -0.616f, -0.602f, -0.588f, -0.574f, -0.559f, -0.545f, -0.530f, -0.515f, 
-0.500f, -0.485f, -0.469f, -0.454f, -0.438f, -0.423f, -0.407f, -0.391f, -0.375f, -0.358f, 
-0.342f, -0.326f, -0.309f, -0.292f, -0.276f, -0.259f, -0.242f, -0.225f, -0.208f, -0.191f, 
-0.174f, -0.156f, -0.139f, -0.122f, -0.105f, -0.087f, -0.070f, -0.052f, -0.035f, -0.017f};

void setup(){
  init();
  ENCinit();

  //ENCvarTest();
  //ENClinTest();

  DRVwrite(0x02, (1<<3));//Set 3PWM inputs, many other settings

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


  analogWrite(pin_pwm_pha, 1);
  analogWrite(pin_pwm_phb, 1);
  analogWrite(pin_pwm_phc, 1);
  // Immediately change to 0% duty cycle
  FTM0_C1V = 0;
  FTM0_C0V = 0;
  FTM0_C6V = 0;
}

void loop(){
  
  /*uint16_t data = ENCread();
  uint16_t pos = data >> 6;
  uint8_t flags = data & 0x3F;
  Serial.println(pos);
  Serial.println(flags>>1, BIN);
  
  digitalWrite(LED2, !digitalRead(LED2));
  delay(100);*/

  /*static uint32_t power = 0;
  Set_PWM_Duty_Scaled(power, power / 2, power / 4);

  power += 20;
  power %= 65536;
  delay(1);*/

  digitalWrite(LED2, HIGH);

  static int idummy = 300000;
  idummy = 3000000;
  for(uint32_t i = 0; i < 10; i++)
  {
    idummy = idummy / 3;
  }
  
  digitalWrite(LED2, LOW);
  delay(10);
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

void Inv_Clark_Park(float d, float Q, float theta)
{
  
}

