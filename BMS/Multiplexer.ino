uint8_t outputPins[4] = {15, 16, 17, 20};
uint8_t inputPin = 14;

void setup() {
  Serial.begin(9600);
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(outputPins[i], OUTPUT);
  }
  pinMode(inputPin, INPUT);
}

void loop() {
  for (int i = 0; i < 12; i++) {
    checkEachPin(i);
  }
  Serial.println();
  delay(2000);
}

void checkEachPin(int channel) {
  uint8_t digits[4];
  for (uint8_t i = 0; i < 4; i++) {
    digits[i] = bitRead(channel, i);
    for (uint8_t i = 0; i < 4; i++) {
      if (digits[i] > 0) {
        digitalWrite(outputPins[i], HIGH);
      } else {
        digitalWrite(outputPins[i], LOW);
      }
    }
    delay(10);
    Serial.print(analogRead(inputPin));
    Serial.print(" ");
  }
}

