//https://github.com/jrowberg/i2cdevlib/blob/master/Arduino/MPU6050/examples/MPU6050_DMP6_ESPWiFi/MPU6050_DMP6_ESPWiFi.ino
//https://www.geekmomprojects.com/mpu-6050-redux-dmp-data-fusion-vs-complementary-filter/
//http://www.geekmomprojects.com/mpu-6050-dmp-data-from-i2cdevlib/

//TOOLS->Board->ESP8266 Boards (3.0.2)->LOLIN (WeMos) D1 mini Lite

//File → Preferences → ADD: https://arduino.esp8266.com/stable/package_esp8266com_index.json
//Tools → Board → Boards Manager -> esp8266
// Board: LOLIN (WEMOS) D1 mini Lite

//pip install esptool
//esptool --chip esp8266 --port COM4 erase_flash

/****************************************************************************************************************************************************
 * SETUP ENVIROMENT FOR "ESP8266 WeMOS D1 mini" chip:
 * link:
 * 
 * 1) Additional Board Manager URLs: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 * OR: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
 * 2) Tools > Board > Boards Manager -> esp8266
 * 3) TOOLS->Board->ESP8266 Boards (3.0.2)->LOLIN (WEMOS) D1 mini Lite
 * 4) Tools > Port and select the COM port 
 ****************************************************************************************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>
#include <math.h>

/* =========================
   CONFIGURAZIONE OLED
   ========================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* =========================
   MPU6050
   ========================= */
MPU6050 mpu;

/* =========================
   VARIABILI GLOBALI
   ========================= */
float currentG     = 0.0;
float impactG      = 0.0;
float peakG        = 0.0;
float lastPeakG    = 0.0;
float recordPeakG  = 0.0;

float sumPeakG     = 0.0;   // somma picchi colpi
float avgPeakG     = 0.0;   // media colpi

unsigned long lastHitTime = 0;
unsigned int hitCount = 0;

/* =========================
   PARAMETRI
   ========================= */
#define IMPACT_THRESHOLD 1.5
#define HIT_TIMEOUT      50
#define PUNCH_MASS       3.5   // kg

/* =========================
   FUNZIONI
   ========================= */
float impactKg(float gValue) {
  return gValue * PUNCH_MASS;
}

/* =========================
   SETUP
   ========================= */
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(100000);
  Wire.setTimeout(50); 

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Inizializzazione...");
  display.display();

  mpu.initialize();
  if (!mpu.testConnection()) {
    display.clearDisplay();
    display.println("MPU6050 ERRORE");
    display.display();
    while (1);
  }

  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);

  display.clearDisplay();
  display.println("Sistema pronto");
  display.display();

  delay(1000);
}

/* =========================
   LOOP
   ========================= */
void loop() {
  static unsigned long lastDisplayUpdate = 0;
  const unsigned long DISPLAY_INTERVAL = 30;

  int16_t axRaw, ayRaw, azRaw;
  float ax, ay, az;

  /* =========================
     LETTURA MPU (PROTETTA)
     ========================= */
  if (!mpu.testConnection()) {
    // se l'MPU non risponde, salta questo ciclo
    return;
  }

  mpu.getAcceleration(&axRaw, &ayRaw, &azRaw);

  // Protezione da letture invalide
  if (axRaw == 0 && ayRaw == 0 && azRaw == 0) {
    return;
  }

  ax = axRaw / 2048.0;
  ay = ayRaw / 2048.0;
  az = azRaw / 2048.0;

  currentG = sqrt(ax * ax + ay * ay + az * az);

  if (isnan(currentG) || currentG > 20.0) {
    return; // scarta valori assurdi
  }

  impactG = fabs(currentG - 1.0);

  /* =========================
     RILEVAMENTO COLPO
     ========================= */
  if (impactG > IMPACT_THRESHOLD) {
    if (impactG > peakG) peakG = impactG;
    lastHitTime = millis();
  }

  /* =========================
     FINE COLPO
     ========================= */
  if (peakG > 0 && (millis() - lastHitTime) > HIT_TIMEOUT) {
    lastPeakG = peakG;

    if (lastPeakG > recordPeakG) {
      recordPeakG = lastPeakG;
    }

    sumPeakG += lastPeakG;
    hitCount++;
    avgPeakG = sumPeakG / hitCount;

    peakG = 0;
  }

  /* =========================
     OLED (REFRESH CONTROLLATO)
     ========================= */
  if (millis() - lastDisplayUpdate < DISPLAY_INTERVAL) return;
  lastDisplayUpdate = millis();

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("SHOT ANALYZER");

  display.setCursor(90, 0);
  display.print("H:");
  display.print(hitCount);

  // Ultimo colpo
  display.setTextSize(3);
  display.setCursor(0, 18);
  display.print(impactKg(lastPeakG), 1);
  display.print("kg");

  // Media colpi
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("AVG:");
  display.print(impactKg(avgPeakG), 1);

  // Record
  display.setCursor(80, 50);
  display.print("MAX:");
  display.print(impactKg(recordPeakG), 1);

  display.display();
}