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
   VARIABILI
   ========================= */
float currentG   = 0.0;   // accelerazione istantanea
float impactG    = 0.0;   // accelerazione senza gravità
float peakG      = 0.0;   // picco del colpo in corso
float lastPeakG  = 0.0;   // ultimo colpo registrato

unsigned long lastHitTime = 0;

#define IMPACT_THRESHOLD 1.5   // g sopra la gravità (soglia colpo)
#define HIT_TIMEOUT      50    // ms fine colpo

/* =========================
   SETUP
   ========================= */
void setup() {
  Serial.begin(115200);
  Wire.begin(); // ESP8266: SDA=D2, SCL=D1

  // --- OLED INIT ---
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Errore OLED");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Inizializzazione...");
  display.display();

  // --- MPU6050 INIT ---
  mpu.initialize();
  if (!mpu.testConnection()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MPU6050 ERRORE");
    display.display();
    while (1);
  }

  // Imposta range accelerometro ±16g
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sistema pronto");
  display.display();

  delay(1000);
}

/* =========================
   LOOP
   ========================= */
void loop() {
  int16_t axRaw, ayRaw, azRaw;
  float ax, ay, az;

  // Lettura accelerazione grezza
  mpu.getAcceleration(&axRaw, &ayRaw, &azRaw);

  // Conversione in g (±16g → 2048 LSB/g)
  ax = axRaw / 2048.0;
  ay = ayRaw / 2048.0;
  az = azRaw / 2048.0;

  // Modulo vettoriale accelerazione totale
  currentG = sqrt(ax * ax + ay * ay + az * az);

  // Accelerazione dinamica (rimozione gravità)
  impactG = fabs(currentG - 1.0);

  // Rilevamento colpo
  if (impactG > IMPACT_THRESHOLD) {
    if (impactG > peakG) {
      peakG = impactG;
    }
    lastHitTime = millis();
  }

  // Fine colpo → salva massimo
  if (peakG > 0 && (millis() - lastHitTime) > HIT_TIMEOUT) {
    lastPeakG = peakG;
    peakG = 0;
  }

  // DEBUG seriale
  Serial.print("Attuale: ");
  Serial.print(impactG, 2);
  Serial.print(" g | Ultimo max: ");
  Serial.print(lastPeakG, 2);
  Serial.println(" g");

  // OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Colpo al sacco");

  display.setCursor(0, 16);
  display.print("Attuale: ");
  display.print(impactG, 2);
  display.println(" g");

  display.setCursor(0, 32);
  display.print("Ultimo max:");
  display.print(lastPeakG, 2);
  display.println(" g");

  display.display();

  delay(30); // ~33 Hz
}

