#define HALL1 16
#define HALL2 17
#define HALL3 20

void setup() {
  Serial.begin(9600);
  
  pinMode(HALL1, INPUT);
  pinMode(HALL2, INPUT);
  pinMode(HALL3, INPUT);

}

void loop() {
  
  int out1 = digitalRead(HALL1);
  int out2 = digitalRead(HALL2);
  int out3 = digitalRead(HALL3);
  
  Serial.print(out1);
  Serial.print("\t");
  Serial.print(out2);
  Serial.print("\t");
  Serial.print(out3);
  Serial.print("\t");
  Serial.print("\n");

  delay(100);
}
