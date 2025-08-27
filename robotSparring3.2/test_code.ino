// ESP32 + PCA9685 (Adafruit_PWMServoDriver) + 4x RDS3120
// Movimento: 25° <-> 100° alla massima velocità (senza ramp)

// Librerie:
//  - Adafruit PWM Servo Driver: https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library
//  - Adafruit BusIO (dipendenza)

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// --- CONFIGURAZIONE HARDWARE ---
static const uint8_t I2C_SDA  = 21;
static const uint8_t I2C_SCL  = 22;
static const uint32_t I2C_HZ  = 1'000'000; // prova 1 MHz; se instabile, scendi a 400'000

// Indirizzo PCA9685 (default 0x40 con A0..A5 a 0)
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Canali da usare per pilotare i motori (0..15)
uint8_t LEFT_STRAIGHT  = 0;
uint8_t LEFT_HOOK      = 1;
uint8_t RIGHT_HOOK	   = 14;
uint8_t RIGHT_STRAIGHT = 15;

// PARAMETRI SERVO (RDS3120 o BLS-HV20KG-180)
// P.S. Se noti che a 0° o 180° tocca meccanicamente, riduci un po’ i limiti (es. 600–2400 µs)
static const float SERVO_MIN_US = 500.0f;
static const float SERVO_MAX_US = 2500.0f;  
static const float SERVO_NEU_US = 1500.0f;

// Impostiamo 50 Hz per servi classici
static const float SERVO_FREQ_HZ = 50.0f;

// Limiti di movimento richiesti
static const float ANGLE_A = 25.0f;
static const float ANGLE_B = 100.0f;

// RDS3120 
// Velocità dichiarata ~0.17 s/60° @6.8V -> ~2.83 ms/°; per 75° ≈ 212 ms + un margine.
static const uint32_t MOVE_WAIT_MS = 250;

// BLS-HV20KG-180
// Velocità dichiarata ~0.06 s / 60° @7,4–8,4V => ~1 ms/°, per 75° -> ~75 ms + un margine.
//static const uint32_t MOVE_WAIT_MS = 120;

// UTILITY: conversioni ---
inline uint16_t usToTicks(float us, float pwmHz) {
  // PCA9685: 12 bit (0..4095), periodo = 1/pwmHz (es. 20'000 us @ 50 Hz)
  const float period_us = 1e6f / pwmHz;
  float ticks = (us / period_us) * 4096.0f; // 4096 step per periodo
  if (ticks < 0)   ticks = 0;
  if (ticks > 4095) ticks = 4095;
  return (uint16_t)(ticks + 0.5f);
}

inline float clamp(float v, float lo, float hi) {
  return (v < lo) ? lo : (v > hi ? hi : v);
}

// Converte gradi [0..180] in microsecondi, rimappando linearmente tra SERVO_MIN_US e SERVO_MAX_US
float angleDegToUs(float deg) {
  deg = clamp(deg, 0.0f, 180.0f);
  // 0° -> MIN_US, 180° -> MAX_US
  return SERVO_MIN_US + (SERVO_MAX_US - SERVO_MIN_US) * (deg / 180.0f);
}

// Scrive l’angolo su un canale
void writeAngle(uint8_t ch, float deg) {
  float us = angleDegToUs(deg);
  uint16_t off = usToTicks(us, SERVO_FREQ_HZ);
  // on=0, off=calcolato: impulso “alto” da 0 a off
  pwm.setPWM(ch, 0, off);
}

// Muove tutti i 4 servi allo stesso angolo
void moveAllTo(float deg) {
  writeAngle(LEFT_STRAIGHT,  deg);
  writeAngle(RIGHT_STRAIGHT, deg);
  writeAngle(LEFT_HOOK, 	 deg);
  writeAngle(RIGHT_HOOK, 	 deg);
}

void setup() {
  // I²C ad alta velocità
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(I2C_HZ);

  // Avvio PCA9685
  pwm.begin();
  pwm.setOscillatorFrequency(25000000); // 25 MHz tipico
  pwm.setPWMFreq(SERVO_FREQ_HZ);
  delay(10);

  // Porta i servi a 25° come posizione iniziale
  moveAllTo(ANGLE_A);
  delay(300);
}

void loop() {
  // 25° -> 100°
  moveAllTo(ANGLE_B);
  delay(MOVE_WAIT_MS);

  // 100° -> 25°
  moveAllTo(ANGLE_A);
  delay(MOVE_WAIT_MS);
}
