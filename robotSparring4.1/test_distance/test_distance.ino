//https://randomnerdtutorials.com/esp32-hc-sr04-ultrasonic-arduino/
//ESP32S3 Dev Module
//Esp32 S3 DevKitC-1

#include <Adafruit_NeoPixel.h>

const int trigPin = 43;
const int echoPin = 44;

float threshold = 60.0; // soglia in cm

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

// LED
#define LED_PIN 48      // Pin del LED RGB
#define NUM_LEDS 1      // C’è solo un LED
Adafruit_NeoPixel led(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200); 
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);  

  led.begin(); 
  led.show();

  Serial.println("Start");
}

// Funzione di attesa non bloccante interrotta da oggetto vicino
bool watchDelay(long waitTime) {
  unsigned long startMillis = millis();
  unsigned long lastSensorRead = 0;
  const long sensorInterval = 100; // ms

  while (millis() - startMillis < waitTime) {
    if (millis() - lastSensorRead >= sensorInterval) {
      lastSensorRead = millis();

      // Trigger ultrasuoni
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      // Echo
      duration = pulseIn(echoPin, HIGH, 20000); // timeout 20 ms
      if (duration > 0) {
        distanceCm = duration * SOUND_SPEED / 2.0;
        distanceInch = distanceCm * CM_TO_INCH;

        Serial.print("Distance (cm): ");
        Serial.println(distanceCm);

        // Controllo LED
        if (distanceCm < threshold) {
          led.setPixelColor(0, led.Color(255, 0, 0)); // rosso
          led.show();
          Serial.println("Oggetto rilevato! Esco da myDelay.");
          return true;  // uscita anticipata
        } else {
          led.setPixelColor(0, led.Color(0, 0, 0)); // spento
          led.show();
        }
      }
    }
  }
  return false; // uscita per tempo scaduto
}

void loop() {
  bool earlyExit = watchDelay(5000); // attende max 5 sec o meno
  if (earlyExit) {
    Serial.println("Loop: rilevato oggetto sotto soglia!");
  } else {
    Serial.println("Loop: attesa terminata senza rilevamenti.");
  }

  delay(1000); // pausa prima del prossimo ciclo
}
