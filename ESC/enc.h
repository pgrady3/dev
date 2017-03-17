#define ENC_A 1
#define ENC_B 2
#define ENC_CS 0

void setupEnc(void)
{
  pinMode(ENC_CS, OUTPUT);
  digitalWriteFast(ENC_CS, HIGH);
  digitalWriteFast(ENC_CS, LOW);
  digitalWriteFast(ENC_CS, HIGH);

  delay(100);//not sure if we need this

  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);
}