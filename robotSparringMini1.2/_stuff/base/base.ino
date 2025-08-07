void setup() {
  Serial.begin(115200);
  delay(2000);  // Dai tempo al monitor di iniziare
  Serial.println("Sketch pulito caricato correttamente!");
}

void loop() {
  delay(1000);
  Serial.println("Loop ok");
}
