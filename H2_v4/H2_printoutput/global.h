#define LED1 10
#define LED2 11
#define LED3 13

#define SUPPLY_VALVE 5
#define PURGE_VALVE 6
#define FAN 4
#define FAN_READ 17
#define THERM 21

#define BUZZER 9
#define SHORT_CIRCUIT 8
#define PASS 3


double fanSpeed = .4;
double voltage = 0;
double current = 0;
double power = 0;
double fan_current = 0;
float prct = 0;
double temp = 0;
double health = 0;
float totFCNRG = 0;
float allEff = 0;
float totH2NRG = 0;
float flowPres = 0;
float BMSCurrent = 0;

bool usingLoadShort = true;
bool currentlyShorting = false;

float massFlow[2] = {0,0};

const uint32_t Short_StartupIntervals[6] = {0, 616, 616, 616, 313, 313};
//const uint32_t Short_StartupDurations[6] = {50, 50, 50, 50, 100, 50};
const uint32_t Short_StartupDurations[6] = {20, 20, 20, 20, 20, 20};//modified from original so controller doesn't need supercaps to boot successfully
uint8_t Short_StartupIndex = 0;

int short_start = 999999999;
int start_Purge_delay = 999999999;
int start_Purge = 999999999;

#define EFF_AVG_SIZE 120 //called every 500ms
uint32_t effAvgPos = 0;
float effAvgEnergy[EFF_AVG_SIZE];
float effAvgTotalFlow[EFF_AVG_SIZE];
float effAvg = 0;


float integrateEnergy(float v, float i)
{
  static uint32_t lastIntegrationMicros = micros();
  uint32_t curMicros = micros();
  
  float dt = (curMicros - lastIntegrationMicros)/1e6;
  float p = v*i;
  totFCNRG = totFCNRG + p*dt;

  lastIntegrationMicros = curMicros;

  return totFCNRG;
}

void FCShort(uint32_t duration)
{
  digitalWrite(PASS, LOW);
  delay(1);
  digitalWrite(SHORT_CIRCUIT, HIGH);
  delay(duration);
  digitalWrite(SHORT_CIRCUIT, LOW);
  delay(2);
  digitalWrite(PASS, HIGH);
}


void FCShort_Start()
{
  digitalWrite(BUZZER, HIGH);
  short_start = millis();
  currentlyShorting = true;

  integrateEnergy(voltage, current);
  digitalWrite(PASS, LOW);
  delay(1);
  digitalWrite(SHORT_CIRCUIT, HIGH);
}

void FCShort_Stop()
{
  digitalWrite(BUZZER, LOW);
  digitalWrite(SHORT_CIRCUIT, LOW);
  delay(2);
  digitalWrite(PASS, HIGH);

  if(currentlyShorting)
  {
    integrateEnergy(0, 0);
    currentlyShorting = false;
  }
}

void FCPurge_Start()
{
  start_Purge_delay = 999999999;
  start_Purge = millis();
  digitalWrite(PURGE_VALVE,HIGH);
}

void FCPurge_Stop()
{
  digitalWrite(PURGE_VALVE,LOW);
}


