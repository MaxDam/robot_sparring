//https://randomnerdtutorials.com/esp32-hc-sr04-ultrasonic-arduino/
//https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
//ESP32S3 Dev Module
//Esp32 S3 DevKitC-1

#include <Adafruit_NeoPixel.h>

const int trigPin = 43;
const int echoPin = 44;

float threshold = 60.0;

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
  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  
  led.begin(); // Inizializza il LED
  led.show();
  //led.setPixelColor(0, led.Color(255, 0, 0)); // rosso
  //led.show();

  Serial.print("Start");
}

void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);

  if(distanceCm < threshold) {
    led.setPixelColor(0, led.Color(255, 0, 0));
  } else {
    led.setPixelColor(0, led.Color(0, 0, 0));
  }
  led.show();
  
  delay(100);
}
