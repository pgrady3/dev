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


float massFlow[2] = {0,0};

const uint32_t Short_StartupIntervals[6] = {0, 616, 616, 616, 313, 313};
const uint32_t Short_StartupDurations[6] = {50, 50, 50, 50, 100, 50};
uint8_t Short_StartupIndex = 0;

int short_start = 999999999;
int start_Purge_delay = 999999999;
int start_Purge = 999999999;

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
  
  short_start = millis();
  digitalWrite(PASS, LOW);
  delay(1);
  digitalWrite(SHORT_CIRCUIT, HIGH);
}

void FCShort_Stop()
{
  
  digitalWrite(SHORT_CIRCUIT, LOW);
  delay(2);
  digitalWrite(PASS, HIGH);
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


