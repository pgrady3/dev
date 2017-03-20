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

float command_D = 0.07f;
float command_Q = 0.0f;

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
  static float angle = 0.0f;

  digitalWrite(LED2, HIGH);

  angle += 21.6f;
  if(angle >= 360.0f)
  {
    angle = 0.0f;
  }
  
  float sin_theta = sin(angle*3.14159f/180.0f);
  float cos_theta = cos(angle*3.14159f/180.0f);
  float alpha = command_D*cos_theta - command_Q*sin_theta;
  float beta = command_Q*cos_theta + command_D*sin_theta;

  float phA = alpha;
  float phB = -0.5f*alpha + beta*0.866f; // 0.866 is about sqrt(3)/2
  float phC = -0.5f*alpha - beta*0.866f;

  float tA = (phA + 0.5/* - minP*/)*0.577f; // 0.577 is 1/sqrt(3)
  float tB = (phB + 0.5/* - minP*/)*0.577f;
  float tC = (phC + 0.5/* - minP*/)*0.577f;

  Set_PWM_Duty_Scaled((long)(tA*65536.0f), (long)(tB*65536.0f), (long)(tC*65536.0f));  
  
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

