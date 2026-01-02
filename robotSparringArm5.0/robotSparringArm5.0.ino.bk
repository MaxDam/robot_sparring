// Board: Espressif ESP32-C6-DevKitC-1
// Framework: Espidf

//http://arduino.esp8266.com/stable/package_esp8266com_index.json
//https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

/******************************************************************************************************************************************************************
Board:
Per ESP32 :   Tools->Board->esp32->DOIT ESP32 DEVKIT V1
Per ESP32 S3: Tools->Board->esp32->ESP32S3 Dev Module

Librerie da installare:
- Adafruit PWM Servo Driver Library 3.0.2 by Adafruit
- Adafruit BusIO 1.17.2 by Adafruit (dipendenza automatica)
- ElegantOTA 3.1.7 by Ayush Sharma
- ESP32Servo 3.0.9 by K.Harrington, J.K.Bennet

Per compilare in modalità OTA (Over-the-Air):
articolo di riferimento: https://randomnerdtutorials.com/esp32-ota-elegantota-arduino/
1) Compilare il codice: Sketch->Verify/Compile (Crtl+R)
2) Esportare il compilato: Sketch->Export Compiled Binary (Alt+Ctrl+S) 
3) Il file xxx.ino.bin generato verrà salvato nella cartella del progetto (robotSparring4.0\build\esp32.esp32.esp32s3\robotSparring4.0.ino.bin)
4) Collegarsi alla rete dell'ESP32 Robot-Sparring-AP - 12345678(
5) Andare all'indirizzo: http://192.168.4.1/update ed effettuare l'upload del file xxx.ino.bin
******************************************************************************************************************************************************************/


/****************************************************************************************************************************************************
 * SETUP ENVIROMENT FOR "ESP-WROOM-32 38 PIN Develeopment" chip:
 * link: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
 *
 * 1) Additional Board Manager URLs: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 * OR: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
 * 2) Tools > Board > Boards Manager -> install "ESP32 by Espressif Systems"
 * 3) TOOLS->Board->ESP32 Arduino->DOIT ESP32 DEVKIT V1
 * 4) Tools > Port and select the COM port 
 ****************************************************************************************************************************************************/

 
#include "WiFi.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <math.h>
#include <ElegantOTA.h>

// IC2 Configuration PCA9685
static const uint8_t I2C_SDA  = 21;
static const uint8_t I2C_SCL  = 22;
static const uint32_t I2C_HZ  = 400'000;

// WiFi and AP
const char* ssid     = "Robot-Sparring-AP";
const char* password = "12345678";
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

//servo driver calibration
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Servo Params (BLS-HV20KG-180)
static const int SERVO_MIN = 80;
static const int SERVO_MAX = 600;

// Set 50 Hz for classic servo
static const float SERVO_FREQ_HZ = 50.0f;


//joint
#define ELBOW_LEFT	               0
#define SHOULDER_FRONT_LEFT        1
#define SHOULDER_ROTATION_LEFT	   2
#define ELBOW_RIGHT	               4
#define SHOULDER_FRONT_RIGHT       5
#define SHOULDER_ROTATION_RIGHT	   6


//level (type of combination)
#define STOP            0
#define START           1
#define SINGLE_SHOT     2
#define DOUBLE_SHOT     3
#define TRIPLE_SHOT     4
#define QUADRUPLE_SHOT  5
#define STRAIGHT_ONLY   6
#define HOOK_ONLY       7
#define FIX_COMBO_1_2   8
#define FIX_COMBO_2_3   9
#define FIX_COMBO_1_2_3 10
#define FIX_COMBO_3_4   11

unsigned int level = START;


//actions (type of shot)
#define NO_ACTION       	 0
#define SINGLE_ACTION      1
#define DOUBLE_ACTION      2
#define TRIPLE_ACTION      3
#define QUADRUPLE_ACTION   4
#define STRAIGHT_ACTION    5
#define HOOK_ACTION        6
#define COMBO_1_2_ACTION   7
#define COMBO_2_3_ACTION   8
#define COMBO_1_2_3_ACTION 9
#define COMBO_3_4_ACTION   10


//speed (shot speed)
#define SLOW            0
#define FAST            1
#define VERYFAST        2
unsigned int speed = VERYFAST;


//pause level (pause random duration)
#define NEVER        0
#define BYRANGE      1 //check adversarial attack (by radar sensor)
#define SHORT        2
#define MEDIUM       3
#define LONG         4
unsigned int shotPause = NEVER;


//stance (orthodox or southpaw)
#define RANDOM_STANCE   0
#define ORTHODOX_STANCE 1
#define SOUTHPAW_STANCE 2
unsigned int stance = RANDOM_STANCE;
bool stanceIsSouthpaw = false;


//other properties
unsigned long shotCount = 0;

//Servo wait
int pauseMax = 1000;

// BLS-HV20KG-180
// Declarate speed ~0.06 s / 60° @7,4–8,4V => ~1 ms/°, per 75° -> ~75 ms + a margin.
int shotDuration = 500;


// DINSTANCE SR04
const int trigPin = 5;
const int echoPin = 18;

float threshold = 60.0; // soglia in cm

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

//radar parameters
long duration;
float distanceCm;
float distanceInch;


// CONFIGURATION WEB PAGE

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="it">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Robot Sparring Pro</title>
  <style>
    body { 
      background-color: #2c2c2c; 
      color: #e0e0e0; 
      font-family: Arial, sans-serif; 
      text-align: center; 
      padding: 0 20px; 
    }
    h1 { font-size: 2.2rem; color: #fff; margin-bottom: 20px; }

    .tabs { display: flex; justify-content: center; margin-bottom: 30px; }
    .tab-btn { 
      background: #444; color: #fff; border: none; 
      padding: 18px 40px; cursor: pointer; font-size: 1.5rem; margin: 0 8px; 
      border-radius: 16px 16px 0 0; 
    }
    .tab-btn.active { background: #fff; color: #222; }

    .tab-content { background: #222; padding: 30px 10px; border-radius: 0 0 18px 18px; }

    label { font-size: 1.4rem; margin: 18px 0 8px 0; display: block; }
    select { 
      font-size: 1.3rem; padding: 14px 18px; border-radius: 12px; border: 1px solid #888; 
      margin-bottom: 18px; width: 90%; max-width: 400px; 
    }

    input[type="range"] {
      width: 90%; margin-top: 30px; appearance: none; height: 24px;
      background: #444; border-radius: 12px; outline: none; padding: 0;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none; appearance: none; width: 36px; height: 36px; 
      border-radius: 50%; background: #ffffff; cursor: pointer;
      box-shadow: 0 0 4px rgba(0,0,0,0.5); margin-top: -6px;
    }
    input[type="range"]::-moz-range-thumb {
      width: 36px; height: 36px; border-radius: 50%; background: #ffffff; cursor: pointer;
      box-shadow: 0 0 4px rgba(0,0,0,0.5);
    }

    button {
      margin: 16px; font: inherit; background-color: #f0f0f0; border: 0; color: #242424;
      border-radius: 0.5em; font-size: 1.5rem; padding: 18px 40px; font-weight: 700;
      text-shadow: 0 0.0625em 0 #fff;
      box-shadow: inset 0 0.0625em 0 0 #f4f4f4, 0 0.0625em 0 0 #efefef,
                  0 0.125em 0 0 #ececec, 0 0.25em 0 0 #e0e0e0,
                  0 0.3125em 0 0 #dedede, 0 0.375em 0 0 #dcdcdc,
                  0 0.425em 0 0 #cacaca, 0 0.425em 0.5em 0 #cecece;
      transition: 0.15s ease; cursor: pointer;
    }
    button:active {
      translate: 0 0.225em;
      box-shadow: inset 0 0.03em 0 0 #f4f4f4, 0 0.03em 0 0 #efefef,
        0 0.0625em 0 0 #ececec, 0 0.125em 0 0 #e0e0e0, 0 0.125em 0 0 #dedede,
        0 0.2em 0 0 #dcdcdc, 0 0.225em 0 0 #cacaca, 0 0.225em 0.375em 0 #cecece;
    }
    .slider-value { font-weight: bold; color: #fff; margin-left: 18px; font-size: 1.3rem; }

    /* --- Manual: immagine + pulsanti overlay --- */
    .manual-wrap { display:flex; flex-direction:column; align-items:center; gap:18px; }
    .robot-stage {
      position: relative; display: inline-block; background:#1e1e1e;
      border-radius: 18px; padding: 10px;
      box-shadow: 0 0.425em 0 0 #1a1a1a, 0 0.425em 0.5em 0 #151515;
    }
    .robot-stage img { max-width: 92vw; width: 760px; height: auto; display:block; border-radius: 12px; }

    .arm-btn {
      position: absolute; font: inherit; background: #ffffff; color: #222; border: 0;
      border-radius: 50%; width: 64px; height: 64px; font-weight: 800;
      box-shadow: inset 0 0.0625em 0 0 #f4f4f4, 0 0.0625em 0 0 #efefef,
                  0 0.125em 0 0 #ececec, 0 0.25em 0 0 #e0e0e0,
                  0 0.3125em 0 0 #dedede, 0 0.375em 0 0 #dcdcdc,
                  0 0.425em 0 0 #cacaca, 0 0.425em 0.5em 0 #111;
      transition: 0.15s ease; cursor: pointer; user-select:none;
    }
    .arm-btn:active { translate: 0 0.225em; }

    .arm-left  { left: 4%;   top: 58%; }
    .arm-right { right: 4%;  top: 58%; }
    .arm-ul    { left: 22%;  top: 8%;  }
    .arm-ur    { right: 22%; top: 8%;  }

    @media (max-width: 600px) {
      h1 { font-size: 1.5rem; }
      .tab-btn, button { font-size: 1.1rem; padding: 12px 18px; }
      select, input[type=range] { font-size: 1.1rem; padding: 10px 12px; }
      .arm-btn { width: 54px; height: 54px; font-size: 1.1rem; }
      .arm-left{ left: 3%; top: 59%; }
      .arm-right{ right: 3%; top: 59%; }
      .arm-ul { left: 20%; top: 10%; }
      .arm-ur { right: 20%; top: 10%; }
    }
	
	
	/* Hotspot trasparenti cliccabili sopra l'immagine */
	.hotspot{
	  position:absolute; cursor:pointer; background:rgba(255,255,255,0);
	}
	.hotspot:hover{ outline:2px dashed rgba(255,255,255,.25); border-radius:12px; }

	/* Posizionamento approssimato sui cuscinetti */
	.hs-right  { left: 1.5%;  top: 58%; width: 30%; height: 40%; }
	.hs-left   { right:1.5%;  top: 58%; width: 30%; height: 40%; }
	.hs-ur     { left: 18.5%; top: 4%;  width: 30%; height: 50%; }
	.hs-ul     { right:18.5%; top: 4%;  width: 30%; height: 50%; }

	@media (max-width: 600px){
	  .hs-right  { left: 2%;  top: 59%; }
	  .hs-left   { right:2%;  top: 59%; }
	  .hs-ur     { left: 18%; top: 6%;  }
	  .hs-ul     { right:18%; top: 6%;  }
	}
  </style>
</head>
<body>
  <h1>ROBOT SPARRING PRO</h1>

  <div class="tabs">
    <button class="tab-btn active" onclick="showTab(0)">Actions</button>
    <button class="tab-btn"        onclick="showTab(1)">Manual</button>
    <button class="tab-btn"        onclick="showTab(2)">Config</button>
  </div>

  <!-- TAB 0: ACTIONS -->
  <div id="tab0" class="tab-content">
    <label for="levelSelect">Level:</label>
    <select id="levelSelect">
      <option value="2">Single shot</option>
      <option value="3">Double shot</option>
      <option value="4">Triple shot</option>
      <option value="5">Quadruple shot</option>
			<option value="6">Straight only</option>
			<option value="7">Hook only</option>
			<option value="8">Fix combo 1-2</option>
			<option value="9">Fix combo 2-3</option>
			<option value="10">Fix combo 1-2-3</option>
			<option value="11">Fix combo 3-4</option>
    </select>
    <label for="speedSelect">Speed:</label>
    <select id="speedSelect">
      <option value="2">Very fast</option>
      <option value="1">Fast</option>
      <option value="0">Slow</option>
    </select>
		<label for="pauseSelect">Pause:</label>
    <select id="pauseSelect">
      <option value="4">Long</option>
      <option value="3">Medium</option>
      <option value="2">Short</option>
      <option value="1">By range</option>
    </select>
		<label for="stanceSelect">Stance type:</label>
    <select id="stanceSelect">
      <option value="0">Random</option>
      <option value="1">Orthodox</option>
      <option value="2">Southpaw</option>
    </select>
    <br>
    <button id="startBtn">Start</button>
    <button id="stopBtn">Stop</button>
  </div>

  <!-- TAB 1: MANUAL -->
  <div id="tab1" class="tab-content" style="display:none;">
		<label for="shotSelect">Shot:</label>
    <select id="shotSelect">
      <option value="STRAIGHT_LEFT">Diretto sinistro</option>
      <option value="STRAIGHT_RIGHT">Diretto destro</option>
      <option value="HOOK_LEFT">Gancio sinistro</option>
			<option value="HOOK_RIGHT">Gancio destro</option>
      <option value="UPPERCUT_LEFT">Montante sinistro</option>
			<option value="UPPERCUT_RIGHT">Montante destro</option>
			<option value="UPPERCUT_BODY_LEFT">Montante al corpo sinistro</option>
			<option value="UPPERCUT_BODY_RIGHT">Montante al corpo destro</option>
    </select>
		<br>
		<button id="shotBtn" onclick="shot()">Shot</button>
		<div style="margin-top:20px;">
      <label for="elbowRightLeft">Elbow right</label>
      <input type="range" id="elbowLeft" min="0" max="180" value="90" oninput="setElbowLeft(this.value)"/>
      <span id="elbowLeftValue" class="slider-value">60</span>
    </div>
		<div style="margin-top:20px;">
      <label for="shoulderFrontLeftRange">Shoulder front left</label>
      <input type="range" id="shoulderFrontLeftRange" min="0" max="180" value="90" oninput="setShouderFrontLeft(this.value)"/>
      <span id="shoulderFrontLeftValue" class="slider-value">60</span>
    </div>
		<div style="margin-top:20px;">
      <label for="shoulderRotationLeftRange">Shoulder rotation left</label>
      <input type="range" id="shoulderRotationLeftRange" min="0" max="180" value="0" oninput="setShouderRotationLeft(this.value)"/>
      <span id="shoulderRotationLeftValue" class="slider-value">60</span>
    </div>
 		<div style="margin-top:20px;">
      <label for="elbowRightRange">Elbow right</label>
      <input type="range" id="elbowRightRange" min="0" max="180" value="90" oninput="setElbowRight(this.value)"/>
      <span id="elbowRightValue" class="slider-value">60</span>
    </div>
		<div style="margin-top:20px;">
      <label for="shoulderFrontRightRange">Shoulder front right</label>
      <input type="range" id="shoulderFrontRightRange" min="0" max="180" value="90" oninput="setShouderFrontRight(this.value)"/>
      <span id="shoulderFrontRightValue" class="slider-value">60</span>
    </div>
		<div style="margin-top:20px;">
      <label for="shoulderRotationRightRange">Shoulder rotation right</label>
      <input type="range" id="shoulderRotationRightRange" min="0" max="180" value="0" oninput="setShouderRotationRight(this.value)"/>
      <span id="shoulderRotationRightValue" class="slider-value">60</span>
    </div>
  </div>
  
  <!-- TAB 2: CONFIG -->
  <div id="tab2" class="tab-content" style="display:none;">
    <div style="margin-top:20px;">
      <label for="thresholdRange">Distance threshold (cm):</label>
      <input type="range" id="thresholdRange" min="10" max="300" value="60" oninput="setThreshold(this.value)"/>
      <span id="thresholdValue" class="slider-value">60</span>
    </div>
    <div style="margin-top:30px;">
      <label for="delaySlider">Servo delay (ms):</label>
      <input type="range" id="delaySlider" min="50" max="1600" value="500" oninput="setDelay(this.value)"/>
      <span id="delayValue" class="slider-value">110</span>
    </div>
    <div style="margin-top:20px;">
      <label for="pauseMaxRange">Pause max (ms):</label>
      <input type="range" id="pauseMaxRange" min="300" max="3000" value="1000" oninput="setPauseMax(this.value)"/>
      <span id="pauseMaxValue" class="slider-value">1000</span>
    </div>
  </div>

  <script>
    function showTab(idx) {
      document.querySelectorAll('.tab-btn').forEach((btn,i)=>btn.classList.toggle('active', i===idx));
      document.getElementById('tab0').style.display = idx===0?'block':'none';
      document.getElementById('tab1').style.display = idx===1?'block':'none';
      document.getElementById('tab2').style.display = idx===2?'block':'none';
    }
    function setPauseMax(val) {
      document.getElementById("pauseMaxValue").innerText = val;
      fetch('/setPauseMax?ms=' + val);
    }
    function setDelay(val) {
      document.getElementById("delayValue").innerText = val;
      fetch('/setDelay?ms=' + val);
    }
    function shot() {
      var speed    = document.getElementById('speedSelect').value;
			var shotType = document.getElementById('shotSelect').value
      fetch('/shot?shotType=' + shotType + '&speed=' + speed); 
    }
    function setThreshold(val) {
      document.getElementById("thresholdValue").innerText = val;
      fetch('/setThreshold?val=' + val); 
    }

		function setElbowLeft(val) {
      document.getElementById("elbowLeftValue").innerText = val;
      fetch('/setElbowLeft?val=' + val);
    }
		function setShouderFrontLeft(val) {
      document.getElementById("shoulderFrontLeftValue").innerText = val;
      fetch('/setShoulderFrontLeft?val=' + val);
    }
		function setShouderRotationLeft(val) {
      document.getElementById("shoulderRotationLeftValue").innerText = val;
      fetch('/setShoulderRotationLeft?val=' + val);
    }
		function setElbowRight(val) {
      document.getElementById("elbowRightValue").innerText = val;
      fetch('/setElbowRight?val=' + val);
    }
		function setShouderFrontRight(val) {
      document.getElementById("shoulderFrontRightValue").innerText = val;
      fetch('/setShoulderFrontRight?val=' + val);
    }
		function setShouderRotationRight(val) {
      document.getElementById("shoulderRotationRightValue").innerText = val;
      fetch('/setShoulderRotationRight?val=' + val);
    }
	
    var running = false;
    document.getElementById('startBtn').onclick = function() {
      if(running) return;
      running = true;
      var level  = document.getElementById('levelSelect').value;
      var speed  = document.getElementById('speedSelect').value;
      var pause  = document.getElementById('pauseSelect').value;
			var stance = document.getElementById('stanceSelect').value;
      fetch('/start?level=' + level + '&speed=' + speed + '&pause=' + pause + '&stance=' + stance);
    };
    document.getElementById('stopBtn').onclick = function() {
      running = false;
      fetch('/stop');
    };
  </script>
</body>
</html>
)rawliteral";

//move servo
void writeAngle(uint8_t ch, float deg) {
	int pulse = map(deg, 0, 180, SERVO_MIN, SERVO_MAX);
	pwm.setPWM(ch, 0, pulse);
}

//init servo
void servoInit() {
	pwm.begin();
  pwm.setOscillatorFrequency(25000000); // 25 MHz tipico
  pwm.setPWMFreq(SERVO_FREQ_HZ);
  delay(10);
}

//init distnace sensor
void distanceSensorInit() {
		pinMode(trigPin, OUTPUT); 
		pinMode(echoPin, INPUT);  
}

//function to initialize the access point
void accessPointInit() {
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("Access Point avviato. IP:");
  Serial.println(myIP);
  dnsServer.start(DNS_PORT, "*", myIP);
}

int angleFromPercent(int startDeg, int endDeg, float percent) {
  if (percent < 0)   percent = 0;
  if (percent > 100) percent = 100;
	float angle = startDeg + (endDeg - startDeg) * (percent / 100.0);
	return round(angle);
}

//basic shots

unsigned int ELBOW_LEFT_START_DEGREE 					     = 140;
unsigned int ELBOW_LEFT_END_DEGREE   				       = 15;
unsigned int SHOULDER_FRONT_LEFT_START_DEGREE      = 90;
unsigned int SHOULDER_FRONT_LEFT_END_DEGREE        = 10;
unsigned int SHOULDER_ROTATION_LEFT_START_DEGREE   = 80;
unsigned int SHOULDER_ROTATION_LEFT_END_DEGREE_CW  = 20;
unsigned int SHOULDER_ROTATION_LEFT_END_DEGREE_CCW = 140;


unsigned int ELBOW_RIGHT_START_DEGREE 				      = 140;
unsigned int ELBOW_RIGHT_END_DEGREE   				      = 15;
unsigned int SHOULDER_FRONT_RIGHT_START_DEGREE      = 90;
unsigned int SHOULDER_FRONT_RIGHT_END_DEGREE        = 10;
unsigned int SHOULDER_ROTATION_RIGHT_START_DEGREE   = 80;
unsigned int SHOULDER_ROTATION_RIGHT_END_DEGREE_CW  = 20;
unsigned int SHOULDER_ROTATION_RIGHT_END_DEGREE_CCW = 140;

void startPositionLeft() {
	writeAngle(ELBOW_LEFT, 					    ELBOW_LEFT_START_DEGREE);
	writeAngle(SHOULDER_FRONT_LEFT,     SHOULDER_FRONT_LEFT_START_DEGREE);
	writeAngle(SHOULDER_ROTATION_LEFT,  SHOULDER_ROTATION_LEFT_START_DEGREE);
}

void startPositionRight() {
	writeAngle(ELBOW_RIGHT, 					  ELBOW_RIGHT_START_DEGREE);
	writeAngle(SHOULDER_FRONT_RIGHT,    SHOULDER_FRONT_RIGHT_START_DEGREE);
	writeAngle(SHOULDER_ROTATION_RIGHT, SHOULDER_ROTATION_RIGHT_START_DEGREE);
}

void startPosition() {
	startPositionLeft();
	startPositionRight();
}

void straightLeft() {
	startPositionLeft();
  writeAngle(ELBOW_LEFT, ELBOW_LEFT_END_DEGREE);
	delay(120);
	writeAngle(SHOULDER_FRONT_LEFT, SHOULDER_FRONT_LEFT_END_DEGREE);
	writeAngle(SHOULDER_ROTATION_LEFT, SHOULDER_ROTATION_LEFT_END_DEGREE_CCW);
	delay(shotDuration);
	startPositionLeft();
}

void straightRight() {
	startPositionRight();
  writeAngle(ELBOW_RIGHT, ELBOW_RIGHT_END_DEGREE);
	delay(120);
	writeAngle(SHOULDER_FRONT_RIGHT, SHOULDER_FRONT_RIGHT_END_DEGREE);
	writeAngle(SHOULDER_ROTATION_RIGHT, SHOULDER_ROTATION_RIGHT_END_DEGREE_CW);
	delay(shotDuration);
	startPositionRight();
}

void hookLeft() {
	startPositionLeft();
	writeAngle(ELBOW_LEFT, angleFromPercent(ELBOW_LEFT_START_DEGREE, ELBOW_LEFT_END_DEGREE, 70));
	writeAngle(SHOULDER_ROTATION_LEFT, SHOULDER_ROTATION_LEFT_END_DEGREE_CCW);
	delay(300);
	writeAngle(SHOULDER_FRONT_LEFT, SHOULDER_FRONT_LEFT_END_DEGREE);
	delay(shotDuration);
	startPositionLeft();
}

void hookRight() {
	startPositionRight();
	writeAngle(ELBOW_RIGHT, angleFromPercent(ELBOW_RIGHT_START_DEGREE, ELBOW_RIGHT_END_DEGREE, 70));
	writeAngle(SHOULDER_ROTATION_RIGHT, SHOULDER_ROTATION_RIGHT_END_DEGREE_CW);
	delay(300);
	writeAngle(SHOULDER_FRONT_RIGHT, SHOULDER_FRONT_RIGHT_END_DEGREE);
	delay(shotDuration);
	startPositionRight();
}

void uppercutLeft() {
	startPositionLeft();
	writeAngle(ELBOW_LEFT, angleFromPercent(ELBOW_LEFT_START_DEGREE, ELBOW_LEFT_END_DEGREE, 55));
	delay(300);
	writeAngle(SHOULDER_FRONT_LEFT, angleFromPercent(SHOULDER_FRONT_LEFT_START_DEGREE, SHOULDER_FRONT_LEFT_END_DEGREE, 70));
	delay(shotDuration);
	startPositionLeft();
}

void uppercutRight() {
	startPositionRight();
	writeAngle(ELBOW_RIGHT, angleFromPercent(ELBOW_RIGHT_START_DEGREE, ELBOW_RIGHT_END_DEGREE, 55));
	delay(300);
	writeAngle(SHOULDER_FRONT_RIGHT, angleFromPercent(SHOULDER_FRONT_RIGHT_START_DEGREE, SHOULDER_FRONT_RIGHT_END_DEGREE, 70));
	delay(shotDuration);
	startPositionRight();
}

void uppercutToBodyLeft() {
	startPositionLeft();
	writeAngle(ELBOW_LEFT, angleFromPercent(ELBOW_LEFT_START_DEGREE, ELBOW_LEFT_END_DEGREE, 65));
	writeAngle(SHOULDER_ROTATION_LEFT, angleFromPercent(SHOULDER_ROTATION_LEFT_START_DEGREE, SHOULDER_ROTATION_LEFT_END_DEGREE_CCW, 15));
	delay(300);
	writeAngle(SHOULDER_FRONT_LEFT, angleFromPercent(SHOULDER_FRONT_LEFT_START_DEGREE, SHOULDER_FRONT_LEFT_END_DEGREE, 50));
	delay(shotDuration);
	startPositionLeft();
}

void uppercutToBodyRight() {
	startPositionRight();
	writeAngle(ELBOW_RIGHT, angleFromPercent(ELBOW_RIGHT_START_DEGREE, ELBOW_RIGHT_END_DEGREE, 65));
	writeAngle(SHOULDER_ROTATION_RIGHT, angleFromPercent(SHOULDER_ROTATION_RIGHT_START_DEGREE, SHOULDER_ROTATION_RIGHT_END_DEGREE_CW, 15));
	delay(300);
	writeAngle(SHOULDER_FRONT_RIGHT, angleFromPercent(SHOULDER_FRONT_RIGHT_START_DEGREE, SHOULDER_FRONT_RIGHT_END_DEGREE, 50));
	delay(shotDuration);
	startPositionRight();
}


//shot combinations

void shot_1() {
	if(!stanceIsSouthpaw) {
		straightLeft();
	} else {
		straightRight();
	}
}

void shot_2() {
	if(!stanceIsSouthpaw) {
		straightRight();
	} else {
		straightLeft();
	}
}

void shot_3() {
	if(!stanceIsSouthpaw) {
		hookLeft();
	} else {
		hookRight();
	}
}

void shot_4() {
	if(!stanceIsSouthpaw) {
		hookRight();
	} else {
		hookLeft();
	}
}

void shot_1_2() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		straightRight();
	} else {
		straightRight();
		straightLeft();
	}
}

void shot_2_3() {
	if(!stanceIsSouthpaw) {
		straightRight();
		hookLeft();
	} else {
		straightLeft();
		hookRight();
	}
}

void shot_1_1() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		delay(shotDuration);
		straightLeft();
	} else {
		straightRight();
		delay(shotDuration);
		straightRight();
	}
}

void shot_2_2() {
	if(!stanceIsSouthpaw) {
		straightRight();
		delay(shotDuration);
		straightRight();
	} else {
		straightLeft();
		delay(shotDuration);
		straightLeft();
	}
}

void shot_3_2() {
	if(!stanceIsSouthpaw) {
		hookLeft();
		straightRight();
	} else {
		hookRight();
		straightLeft();
	}
}

void shot_1_3() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		hookLeft();
	} else {
		straightRight();
		hookRight();
	}
}

void shot_3_3() {
	if(!stanceIsSouthpaw) {
		hookLeft();
		delay(shotDuration);
		hookLeft();
	} else {
		hookRight();
		delay(shotDuration);
		hookRight();
	}
}

void shot_4_4() {
	if(!stanceIsSouthpaw) {
		hookRight();
		delay(shotDuration);
		hookRight();
	} else {
		hookLeft();
		delay(shotDuration);
		hookLeft();
	}
}

void shot_3_4() {
	if(!stanceIsSouthpaw) {
		hookLeft();
		delay(shotDuration);
		hookRight();
	} else {
		hookRight();
		delay(shotDuration);
		hookLeft();
	}
}

void shot_1_2_3() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		straightRight();
		hookLeft();
	} else {
		straightRight();
		straightLeft();
		hookRight();
	}
}

void shot_2_3_2() {
	if(!stanceIsSouthpaw) {
		straightRight();
		hookLeft();
		straightRight();
	} else {
		straightLeft();
		hookRight();
		straightLeft();
	}
}

void shot_1_3_1() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		hookLeft();
		straightLeft();
	} else {
		straightRight();
		hookRight();
		straightRight();
	}
}

void shot_1_2_2() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		straightRight();
		delay(shotDuration);
		straightRight();
	} else {
		straightRight();
		straightLeft();
		delay(shotDuration);
		straightLeft();
	}
}

void shot_1_1_2() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		delay(shotDuration);
		straightLeft();
		straightRight();
	} else {
		straightRight();
		delay(shotDuration);
		straightRight();
		straightLeft();
	}
}

void shot_1_2_1() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		straightRight();
		straightLeft();
	} else {
		straightRight();
		straightLeft();
		straightRight();
	}
}

void shot_1_4_3() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		hookRight();
		delay(shotDuration);
		hookLeft();
	} else {
		straightRight();
		hookLeft();
		delay(shotDuration);
		hookRight();
	}
}

void shot_3_4_2() {
	if(!stanceIsSouthpaw) {
		hookLeft();
		delay(shotDuration);
		hookRight();
		straightRight();
	} else {
		hookRight();
		delay(shotDuration);
		hookLeft();
		straightLeft();
	}
}

void shot_3_4_3() {
	if(!stanceIsSouthpaw) {
		hookLeft();
		delay(shotDuration);
		hookRight();
		delay(shotDuration);
		hookLeft();
	} else {
		hookRight();
		delay(shotDuration);
		hookLeft();
		delay(shotDuration);
		hookRight();
	}
}

void shot_3_1_3() {
	if(!stanceIsSouthpaw) {
		hookLeft();
		straightLeft();
		hookLeft();
	} else {
		hookRight();
		straightRight();
		hookRight();
	}
}

void shot_1_2_3_4() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		straightRight();
		hookLeft();
		delay(shotDuration);
		hookRight();
	} else {
		straightRight();
		straightLeft();
		hookRight();
		delay(shotDuration);
		hookLeft();
	}
}

void shot_1_2_1_2() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		straightRight();
		straightLeft();
		straightRight();
	} else {
		straightRight();
		straightLeft();
		straightRight();
		straightLeft();
	}
}

void shot_1_2_3_2() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		straightRight();
		hookLeft();
		straightRight();
	} else {
		straightRight();
		straightLeft();
		hookRight();
		straightLeft();
	}
}

void shot_3_4_2_1() {
	if(!stanceIsSouthpaw) {
		hookLeft();
		delay(shotDuration);
		hookRight();
		straightRight();
		straightLeft();
	} else {
		hookRight();
		delay(shotDuration);
		hookLeft();
		straightLeft();
		straightRight();
	}
}

void shot_3_4_3_2() {
	if(!stanceIsSouthpaw) {
		hookLeft();
		delay(shotDuration);
		hookRight();
		delay(shotDuration);
		hookLeft();
		straightRight();
	} else {
		hookRight();
		delay(shotDuration);
		hookLeft();
		delay(shotDuration);
		hookRight();
		straightLeft();
	}
}

void shot_3_4_3_3() {
	if(!stanceIsSouthpaw) {
		hookLeft();
		delay(shotDuration);
		hookRight();
		delay(shotDuration);
		hookLeft();
		delay(shotDuration);
		hookLeft();
	} else {
		hookRight();
		delay(shotDuration);
		hookLeft();
		delay(shotDuration);
		hookRight();
		delay(shotDuration);
		hookRight();
	}
}

void shot_1_2_1_1() {
	if(!stanceIsSouthpaw) {
		straightLeft();
		straightRight();
		straightLeft();
		delay(shotDuration);
		straightLeft();
	} else {
		straightRight();
		straightLeft();
		straightRight();
		delay(shotDuration);
		straightRight();
	}
}

//decides the action based on probability
int getRandomActionFromProbability(int probs[], int probSize) {
	int randomNumber = random(100+1);
    int threshold = 0;
    for(int i = 0 ; i < probSize ; i++) {
      threshold += probs[i];
      if(randomNumber < threshold) {
        return i;
      }
    }
    return 0;
}

//get wait time (pause based)
int getRandomWaitTime() {
     
	unsigned int waitingTimeMult = 0;
    
    switch(shotPause) {
      case LONG: {
        int num = 5;
        int timeProbs[num] = {2, 24, 24, 24, 24};
        waitingTimeMult = getRandomActionFromProbability(timeProbs, num);
        return shotDuration + waitingTimeMult * pauseMax;
      }
      case MEDIUM: {
        int num = 4;
        int timeProbs[num] = {4, 32, 32, 32};
        waitingTimeMult = getRandomActionFromProbability(timeProbs, num);
        return shotDuration + waitingTimeMult * pauseMax;
      }
      case SHORT: {
        int num = 3;
        int timeProbs[num] = {8, 46, 46};
        waitingTimeMult = getRandomActionFromProbability(timeProbs, num);
        return shotDuration + waitingTimeMult * pauseMax;
      }
      case BYRANGE: {
        return pauseMax * 5;
      }
      default: {
        return 10;
      }
  }
}

//delay with watch (sensor dinstance)
bool watchDelay(long waitTime) {
  unsigned long startMillis = millis();
  unsigned long lastSensorRead = 0;
  const long sensorInterval = 100; // ms

  while (millis() - startMillis < waitTime) {
    if (millis() - lastSensorRead >= sensorInterval) {
      lastSensorRead = millis();

      // Ultrasonic trigger
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

        // Check dinstance
        if (distanceCm < threshold) {
          Serial.println("Attacco avversario rilevato!");
					if(shotPause == BYRANGE) {
            return true; // uscita anticipata
          }
        } 
      }
    }
  }
  return false; // exit for timeout
}

//get action (random probability) level based
int getActionFromState() {
	int action = NO_ACTION;
	switch(level) {
		case START: {    
			startPosition();
			delay(2000);
			straightRight();
			delay(2000);
			level = STOP;
			action = NO_ACTION;
			break;
		}
		case STOP: {
			action = NO_ACTION;
			break;
		}
		case SINGLE_SHOT: {
			action = SINGLE_ACTION;
			break;
		}
    case DOUBLE_SHOT: {
			int num = 2;
			int shotNumberProbs[num] = {40, 60};
			switch(getRandomActionFromProbability(shotNumberProbs, num)) {
  			case 0: action = SINGLE_ACTION; break;
  			case 1: action = DOUBLE_ACTION; break;
  		}
			break;
		}
		case TRIPLE_SHOT: {
			int num = 3;
			int shotNumberProbs[num] = {30, 30, 40};
			switch(getRandomActionFromProbability(shotNumberProbs, num)) {
  			case 0: action = SINGLE_ACTION; break;
  			case 1: action = DOUBLE_ACTION; break;
				case 2: action = TRIPLE_ACTION; break;
  		}
			break;
		}
		case QUADRUPLE_SHOT: {
			int num = 4;
			int shotNumberProbs[num] = {10, 20, 35, 35};
			switch(getRandomActionFromProbability(shotNumberProbs, num)) {
  			case 0: action = SINGLE_ACTION;    break;
  			case 1: action = DOUBLE_ACTION;    break;
				case 2: action = TRIPLE_ACTION;    break;
				case 3: action = QUADRUPLE_ACTION; break;
  		}
			break;
		}
		case STRAIGHT_ONLY: {
			action = STRAIGHT_ACTION;
			break;
		}
		case HOOK_ONLY: {
			action = HOOK_ACTION;
			break;
		}
		case FIX_COMBO_1_2: {
			action = COMBO_1_2_ACTION;
			break;
		}
		case FIX_COMBO_2_3: {
			action = COMBO_2_3_ACTION;
			break;
		}
		case FIX_COMBO_1_2_3: {
			action = COMBO_1_2_3_ACTION;
			break;
		}
		case FIX_COMBO_3_4: {
			action = COMBO_3_4_ACTION;
			break;
		}
	}
	return action;
}

//set or change stance based on probability
void checkStance() {
	switch(stance) {
			case RANDOM_STANCE: {
				int num = 2;
				int changeSideProbs[num] = {80, 20};
				if(getRandomActionFromProbability(changeSideProbs, num) == 0) {
					stanceIsSouthpaw = !stanceIsSouthpaw;
				}
				break;
			}
			case ORTHODOX_STANCE: {
				stanceIsSouthpaw = false;
				break;
			}
			case SOUTHPAW_STANCE: {
				stanceIsSouthpaw = true;
				break;
			}
	}
}


//execute the action (random probability based)
void executeAction(int action)  {
  switch(action) {
	  case NO_ACTION: { //no action
      startPosition();
      break;
	  }
	  case SINGLE_ACTION: { //random single shot
			int num = 4;
  		int singleShotProbs[num] = {25, 25, 25, 25};
  		switch(getRandomActionFromProbability(singleShotProbs, num)) {
  			case 0: shot_1(); break;
  			case 1: shot_2(); break;
				case 2: shot_3(); break;
  			case 3: shot_4(); break;
  		}
      break;
	  }
	  case DOUBLE_ACTION: { //random double shot
			int num = 7;
  		int doubleShotProbs[num] = {15, 14, 14, 14, 15, 14, 14};
  		switch(getRandomActionFromProbability(doubleShotProbs, num)) {
  			case 0: shot_1_2();  break;
  			case 1: shot_3_3();  break;
  			case 2: shot_1_1();  break;
  			case 3: shot_3_2();  break;
  			case 4: shot_1_3();  break;
  			case 5: shot_2_3();  break;
  			case 6: shot_3_4();  break;
  		}
  		break;
	  }
	  case TRIPLE_ACTION: { //random triple shot
			int num = 10;
		  int tripleShotProbs[num] = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
  		switch(getRandomActionFromProbability(tripleShotProbs, num)) {
  			case 0: shot_1_2_3();  break;
  			case 1: shot_3_1_3();  break;
  			case 2: shot_1_3_1();  break;
  			case 3: shot_3_4_2();  break;
  			case 4: shot_1_1_2();  break;
  			case 5: shot_1_2_1();  break;
  			case 6: shot_3_4_3();  break;
  			case 7: shot_1_2_2();  break;
  			case 8: shot_1_4_3();  break;
  			case 9: shot_2_3_2();  break;
  		}
  		break;
	  }
	  case QUADRUPLE_ACTION: { //random quadruple shot
			int num = 7;
  		int quadrupleShotProbs[num] = {15, 14, 14, 14, 15, 14, 14};
  		switch(getRandomActionFromProbability(quadrupleShotProbs, num)) {
  			case 0: shot_1_2_3_4();  break;
  			case 1: shot_3_4_3_3();  break;
  			case 2: shot_1_2_3_2();  break;
  			case 3: shot_3_4_2_1();  break;
  			case 4: shot_1_2_1_2();  break;
  			case 5: shot_3_4_3_2();  break;
  			case 6: shot_1_2_1_1();  break;
  		}
  		break;
	  }
		case STRAIGHT_ACTION: { //random straight shot
			int num = 4;
			int straightOnlyProbs[num] = {45, 45, 5, 5};
  		switch(getRandomActionFromProbability(straightOnlyProbs, num)) {
  			case 0: shot_1();    break;
				case 1: shot_2();    break;
  			case 2: shot_1_1();  break;
				case 3: shot_2_2();  break;
  		}
			break;
		}
		case HOOK_ACTION: { //random hook shot
			int num = 4;
			int hookOnlyProbs[num] = {45, 45, 5, 5};
  		switch(getRandomActionFromProbability(hookOnlyProbs, num)) {
  			case 0: shot_3();    break;
  			case 1: shot_4();    break;
				case 2: shot_3_3();  break;
  			case 3: shot_4_4();  break;
  		}
			break;
		}
		case COMBO_1_2_ACTION: { //fix combo
			shot_1_2();
			break;
		}
		case COMBO_2_3_ACTION: { //fix combo
			shot_2_3();
			break;
		}
		case COMBO_1_2_3_ACTION: { //fix combo
			shot_1_2_3();
			break;
		}
		case COMBO_3_4_ACTION: { //fix combo
			shot_3_4();
			break;
		}
	  default: {
		  startPosition();
	  }
  }

	shotCount++;
}

// Function to handle start request
void handleStart() {
  if (server.hasArg("level"))  level     = server.arg("level").toInt();
  if (server.hasArg("speed"))  speed     = server.arg("speed").toInt();
  if (server.hasArg("pause"))  shotPause = server.arg("pause").toInt();
	if (server.hasArg("stance")) stance    = server.arg("stance").toInt();
  server.send(200, "text/plain", "Started");
}

// Function to handle stop request
void handleStop() {
  level     = STOP;
  shotPause = NEVER;
  server.send(200, "text/plain", "Stopped");
}

// Function to handle delay setting
void handleSetDelay() {
  if (server.hasArg("ms")) {
    int ms = server.arg("ms").toInt();
    shotDuration = ms;
  }
  server.send(200, "text/plain", "OK");
}

// Function to handle pause max setting
void handleSetPauseMax() {
  if (server.hasArg("ms")) {
    int ms = server.arg("ms").toInt();
    pauseMax = ms;
  }
  server.send(200, "text/plain", "OK");
}

// Function to handle manual single shot
void handleShot() {
  if (server.hasArg("shotType")) {
    speed = server.arg("speed").toInt();
    shotPause = NEVER;
    String shotType = server.arg("shotType");
	
		if(shotType == "STRAIGHT_LEFT") {
			straightLeft();
			server.send(200, "text/plain", "OK");
			return;
		}
    if(shotType == "STRAIGHT_RIGHT") {
			straightRight();
			server.send(200, "text/plain", "OK");
			return;
		}
    if(shotType == "HOOK_LEFT") {
			hookLeft();
			server.send(200, "text/plain", "OK");
			return;
		}
		if(shotType == "HOOK_RIGHT") {
			hookRight();
			server.send(200, "text/plain", "OK");
			return;
		}
    if(shotType == "UPPERCUT_LEFT") {
			uppercutLeft();
			server.send(200, "text/plain", "OK");
			return;
		}
		if(shotType == "UPPERCUT_RIGHT") {
			uppercutRight();
			server.send(200, "text/plain", "OK");
			return;
		}
		if(shotType == "UPPERCUT_BODY_LEFT") {
			uppercutToBodyLeft();
			server.send(200, "text/plain", "OK");
			return;
		}
		if(shotType == "UPPERCUT_BODY_RIGHT") {
			uppercutToBodyRight();
			server.send(200, "text/plain", "OK");
			return;
		}

		server.send(200, "text/plain", "OK");
  }
}


// Function to handle threshold
void handleSetThreshold() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    threshold = val;
  }
  server.send(200, "text/plain", "OK");
}


// Function to handleSetElbowLeft
void handleSetElbowLeft() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    writeAngle(ELBOW_LEFT, val);
  }
  server.send(200, "text/plain", "OK");
}
// Function to handleSetShoulderFrontLeft
void handleSetShoulderFrontLeft() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    writeAngle(SHOULDER_FRONT_LEFT, val);
  }
  server.send(200, "text/plain", "OK");
}
// Function to handleSetShoulderRotationLeft
void handleSetShoulderRotationLeft() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    writeAngle(SHOULDER_ROTATION_LEFT, val);
  }
  server.send(200, "text/plain", "OK");
}
// Function to handleSetElbowRight
void handleSetElbowRight() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    writeAngle(ELBOW_RIGHT, val);
  }
  server.send(200, "text/plain", "OK");
}
// Function to handleSetShoulderFrontRight
void handleSetShoulderFrontRight() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    writeAngle(SHOULDER_FRONT_RIGHT, val);
  }
  server.send(200, "text/plain", "OK");
}
// Function to handleSetShoulderRotationRight
void handleSetShoulderRotationRight() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    writeAngle(SHOULDER_ROTATION_RIGHT, val);
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
	Wire.begin(I2C_SDA, I2C_SCL);
	Wire.setClock(I2C_HZ);

	Serial.begin(115200);
	accessPointInit();
	servoInit();
	distanceSensorInit();
  

	server.on("/", HTTP_GET, []() {
    	server.send_P(200, "text/html", index_html);
	});
	server.on("/start",          HTTP_GET, handleStart);
	server.on("/stop",           HTTP_GET, handleStop);
  
	server.on("/setDelay",       HTTP_GET, handleSetDelay);
	server.on("/setPauseMax",    HTTP_GET, handleSetPauseMax);
	server.on("/shot",           HTTP_GET, handleShot);
  server.on("/setThreshold",   HTTP_GET, handleSetThreshold);

	server.on("/setElbowLeft",             HTTP_GET, handleSetElbowLeft);
	server.on("/setShoulderFrontLeft",     HTTP_GET, handleSetShoulderFrontLeft);
	server.on("/setShoulderRotationLeft",  HTTP_GET, handleSetShoulderRotationLeft);
	server.on("/setElbowRight",            HTTP_GET, handleSetElbowRight);
	server.on("/setShoulderFrontRight",    HTTP_GET, handleSetShoulderFrontRight);
	server.on("/setShoulderRotationRight", HTTP_GET, handleSetShoulderRotationRight);
  
	server.onNotFound([]() {
		server.send_P(200, "text/html", index_html);
	});
	
	ElegantOTA.begin(&server);
	server.begin();
}

void loop() {
	//server loops
	dnsServer.processNextRequest();
	server.handleClient();
	ElegantOTA.loop();

	//get random action
	int action = getActionFromState();
	
	//check stance
	checkStance();
	
	//execute action (starts the punch running servo motors)
	executeAction(action);
	
	//wait before striking the next blow shot
	watchDelay(getRandomWaitTime());
}
