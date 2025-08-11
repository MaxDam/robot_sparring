/****************************************************************************************************************************************************
* ROBOT SPARRING MINI
****************************************************************************************************************************************************/

//http://arduino.esp8266.com/stable/package_esp8266com_index.json
//https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

/****************************************************************************************************************************************************
 * SETUP ENVIROMENT FOR "ESP-WROOM-32 38 PIN Develeopment" chip:
 * link: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
 *
 * 1) Additional Board Manager URLs: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 * OR: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json, http://arduino.esp8266.com/stable/package_esp8266com_index.json
 * 2) Tools > Board > Boards Manager -> install "ESP32 by Espressif Systems"
 * 3) TOOLS->Board->ESP32 Arduino->DOIT ESP32 DEVKIT V1
 * 4) Tools > Port and select the COM port 
 

Nota: Per compilare su ESP32 WROOM-32: tenere premuto il pulsante "boot", quando si vede la scritta "Connecting..." rilasciarlo
****************************************************************************************************************************************************/

#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <math.h>

Servo servo;
const int SERVO_PIN       = 17;
const int SERVO_HERTZ     = 50;
const int SERVO_MIN_PULSE = 500;
const int SERVO_MAX_PULSE = 2500;
const int SERVO_MIN_ANGLE = 0;
const int SERVO_MAX_ANGLE = 180;

const char* ssid     = "Robot-Sparring-AP";
const char* password = "12345678";
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='UTF-8' />
  <meta name='viewport' content='width=device-width, initial-scale=1.0' />
  <title>Robot Sparring Control</title>
  <style>
    body { 
      background-color: #2c2c2c; 
      color: #e0e0e0; 
      font-family: Arial, sans-serif; 
      text-align: center; 
      padding: 0 20px; 
    }
    
    h1 { 
      font-size: 2.2rem; 
      color: #fff; 
      margin-bottom: 20px; 
    }
    
    .tabs { 
      display: flex; 
      justify-content: center; 
      margin-bottom: 30px; 
    }
    
    .tab-btn { 
      background: #444; 
      color: #fff; 
      border: none; padding: 18px 40px; 
      cursor: pointer; 
      font-size: 1.5rem; margin: 0 8px; 
      border-radius: 16px 16px 0 0; 
    }
    
    .tab-btn.active { 
      background: #fff;
      color: #222; 
    }
    
    .tab-content { 
      background: #222; 
      padding: 30px 10px;
      border-radius: 0 0 18px 18px; 
    }
    
    label { 
      font-size: 1.4rem; 
      margin: 18px 0 8px 0; 
      display: block; 
    }
    
    select { 
      font-size: 1.3rem; 
      padding: 14px 18px; 
      border-radius: 12px; 
      border: 1px solid #888; 
      margin-bottom: 18px; 
      width: 90%; 
      max-width: 400px; 
    }
    
    input[type="range"] {
      width: 90%;
      margin-top: 30px;
      appearance: none;
      height: 24px;
      background: #444;
      border-radius: 12px;
      outline: none;
      padding: 0;
    }
  
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 36px;
      height: 36px;
      border-radius: 50%;
      background: #ffffff;
      cursor: pointer;
      box-shadow: 0 0 4px rgba(0,0,0,0.5);
      margin-top: -6px;
    }
  
    input[type="range"]::-moz-range-thumb {
      width: 36px;
      height: 36px;
      border-radius: 50%;
      background: #ffffff;
      cursor: pointer;
      box-shadow: 0 0 4px rgba(0,0,0,0.5);
    }

  button {
      margin: 16px;
      font: inherit;
      background-color: #f0f0f0;
      border: 0;
      color: #242424;
      border-radius: 0.5em;
      font-size: 1.5rem;
      padding: 18px 40px;
      font-weight: 700;
      text-shadow: 0 0.0625em 0 #fff;
      box-shadow: inset 0 0.0625em 0 0 #f4f4f4, 0 0.0625em 0 0 #efefef,
                  0 0.125em 0 0 #ececec, 0 0.25em 0 0 #e0e0e0,
                  0 0.3125em 0 0 #dedede, 0 0.375em 0 0 #dcdcdc,
                  0 0.425em 0 0 #cacaca, 0 0.425em 0.5em 0 #cecece;
      transition: 0.15s ease;
      cursor: pointer;
    }
  
    button:active {
      translate: 0 0.225em;
      box-shadow: inset 0 0.03em 0 0 #f4f4f4, 0 0.03em 0 0 #efefef,
        0 0.0625em 0 0 #ececec, 0 0.125em 0 0 #e0e0e0, 0 0.125em 0 0 #dedede,
        0 0.2em 0 0 #dcdcdc, 0 0.225em 0 0 #cacaca, 0 0.225em 0.375em 0 #cecece;
    }
    
    .slider-label { 
      font-size: 1.3rem; 
      margin-top: 20px; 
    }
    
    .slider-value { 
      font-weight: bold; 
      color: #fff; 
      margin-left: 18px; 
      font-size: 1.3rem; 
    }
    
    @media (max-width: 600px) {
      h1 { 
        font-size: 1.5rem; 
      }
      .tab-btn, button { 
        font-size: 1.1rem; 
        padding: 12px 18px; 
      }
      select, input[type=range] { 
        font-size: 1.1rem; 
        padding: 10px 12px; 
      }
    }
  </style>
</head>
<body>
  <h1>ROBOT SPARRING MINI</h1>
  <div class='tabs'>
    <button class='tab-btn active' onclick='showTab(0)'>Actions</button>
    <button class='tab-btn' onclick='showTab(1)'>Manual</button>
  </div>
  <div id='tab0' class='tab-content'>
    <label for='modeSelect'>Mode:</label>
    <select id='modeSelect'>
      <option value='straight'>Straight</option>
      <option value='hook'>Hook</option>
    </select>
    <br>
    <div id='straightActions'>
      <label for='straightActionSelect'>Straight action:</label>
      <select id='straightActionSelect'>
        <option value='random'>Random</option>
        <option value='STRAIGHT'>Straight</option>
        <option value='STRAIGHT_X2'>Double Straight</option>
        <!--
        <option value='STRAIGHT_X3'>Triple Straight</option>
        <option value='FEINT'>Feint</option>
        -->
      </select>
    </div>
    <div id='hookActions' style='display:none;'>
      <label for='hookActionSelect'>Hook action:</label>
      <select id='hookActionSelect'>
        <option value='random'>Random</option>
        <option value='HALF_HOOK'>Half Hook</option>
        <option value='HOOK'>Hook</option>
        <option value='HALF_HOOK_AND_HOOK'>Half Hook & Hook</option>
        <!--
        <option value='HOOK_AND_HALF_HOOK'>Hook & Half Hook</option>
        <option value='HALF_HOOK_X2'>Double Half Hook</option>
        <option value='HOOK_X3'>Triple Hook</option>
        <option value='HOOK_X2'>Double Hook</option>
        <option value='FEINT'>Feint</option>
        -->
      </select>
    </div>
    <br>
    <button id='startBtn'>Start</button>
    <button id='stopBtn'>Stop</button>
    <div style='margin-top:20px;'>
      <label for='pauseRange'>Random pause (ms):</label>
      <input type='range' id='pauseRange' min='1000' max='10000' value='3000' />
      <span id='pauseValue' class='slider-value'>3000</span>
    </div>
  </div>
  <div id='tab1' class='tab-content' style='display:none;'>
    <label for='servoSlider'>Servo angle (0-180):</label>
    <input type='range' id='servoSlider' min='30' max='150' value='90' oninput="setServo(this.value)"/>
    <span id='servoValue' class='slider-value'>90</span>
  </div>
  <script>
    function showTab(idx) {
      document.querySelectorAll('.tab-btn').forEach((btn,i)=>btn.classList.toggle('active',i===idx));
      document.getElementById('tab0').style.display = idx===0?'block':'none';
      document.getElementById('tab1').style.display = idx===1?'block':'none';
    }
    document.getElementById('pauseRange').addEventListener('input', function() {
      document.getElementById('pauseValue').textContent = this.value;
    });
    document.getElementById('servoSlider').addEventListener('input', function() {
      document.getElementById('servoValue').textContent = this.value;
    });
    document.getElementById('modeSelect').addEventListener('change', function() {
      if(this.value === 'straight') {
        document.getElementById('straightActions').style.display = 'block';
        document.getElementById('hookActions').style.display = 'none';
      } else {
        document.getElementById('straightActions').style.display = 'none';
        document.getElementById('hookActions').style.display = 'block';
      }
    });
    function setServo(val) {
      document.getElementById("servoValue").innerText = val;
      fetch('/setServo?angle=' + val);
    }
    var running = false;
    document.getElementById('startBtn').onclick = function() {
      if(running) return;
      running = true;
      var mode = document.getElementById('modeSelect').value;
      var action = mode === 'straight' ? document.getElementById('straightActionSelect').value : document.getElementById('hookActionSelect').value;
      var pause = parseInt(document.getElementById('pauseRange').value);
      fetch('/start?mode=' + mode + '&action=' + action + '&pause=' + pause);
    };
    document.getElementById('stopBtn').onclick = function() {
      running = false;
      fetch('/stop');
    };
  </script>
</body>
</html>
)rawliteral";

int currentPosition = 90;
bool reverseSide = false;

const int POS_START         = 30;
const int POS_END           = 150;
const int POS_MIDDLE        = 90;
const int POS_START_PLUS    = POS_START  + 20;
const int POS_END_MINUS     = POS_END    - 20;
const int POS_MIDDLE_PLUS   = POS_MIDDLE + 20;
const int POS_MIDDLE_MINUS  = POS_MIDDLE - 20;


#define STRAIGHT            0
#define STRAIGHT_X2         1
#define STRAIGHT_X3         2 //EXCLUDED
#define FEINT_STRAIGHT      3 //EXCLUDED  
unsigned int punchStraightType = STRAIGHT;

#define HALF_HOOK           0
#define HOOK                1
#define HALF_HOOK_AND_HOOK  2
#define HOOK_AND_HALF_HOOK  3 //EXCLUDED
#define HALF_HOOK_X2        4 //EXCLUDED
#define HOOK_X3             5 //EXCLUDED
#define HOOK_X2             6 //EXCLUDED
#define FEINT_HOOK          7 //EXCLUDED
unsigned int punchHookType = HALF_HOOK;

//level
#define STOP            0
#define START           1
#define EASY            2
#define MEDIUM          3
#define PRO             4
unsigned int level = START;

//speed
#define VERYFAST        2
#define FAST            1
#define SLOW            0
unsigned int speed = FAST;


volatile bool isRunning = false;
String currentMode = "straight";
String currentAction = "random";
int currentPause    = 3000;
int currentPauseMax = 3000;


void servoInit() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servo.setPeriodHertz(SERVO_HERTZ);
  
  servo.attach(SERVO_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  delay(500);
  servoMove(POS_START_PLUS);
  delay(500);
  servoMove(POS_END_MINUS);
  delay(500);
  servoMove(POS_MIDDLE);
  delay(500);
}

void accessPointInit() {
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("Access Point avviato. IP:");
  Serial.println(myIP);
  dnsServer.start(DNS_PORT, "*", myIP);
}

int getDelay(int posStart, int posEnd) {
  const int DEGREE  = 60;
  const int SPEED   = 140;
  const int MARGIN  = 10;
  const float angle = abs(posEnd - posStart);
  //return ceil(angle * SPEED * 10 / DEGREE + MARGIN);
  return 500; // DA CAPIRE PERCHE' NON FUNZIONA
}

void servoMove(int targetPosition) {
  int servoPulse = map(targetPosition, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  int servoDelay = getDelay(currentPosition, targetPosition);
  currentPosition = targetPosition;

  servo.write(targetPosition);
  delay(servoDelay);
}


void executeStraightAction(unsigned int punchType) {
  Serial.printf("PunchType: %d\n", punchType);
  switch (punchType) {
    case STRAIGHT:
      Serial.println("Executing straight...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START);
      } else {
        servoMove(POS_END);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END);
      }
      break;
    case STRAIGHT_X2:
      Serial.println("Executing double straight...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START);
      } else {
        servoMove(POS_END);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END);
      }
      break;
    /*  
    case STRAIGHT_X3:
      Serial.println("Executing triple straight...");   
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START_PLUS);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START_PLUS);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START);
      } else {
        servoMove(POS_END);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END_MINUS);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END_MINUS);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END);
      }
      break;
    case FEINT_STRAIGHT: 
      Serial.println("Executing feint...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_START);
      } else {
        servoMove(POS_END);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_END);
      }
      break;
    */  
    default:
      Serial.println("Unknown punch type.");
  }
}

void executeHookAction(unsigned int punchType) {
  Serial.printf("PunchType: %d\n", punchType);
  switch (punchType) {
    case HALF_HOOK:
      Serial.println("Executing half hook...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START);
      } else {
        servoMove(POS_END);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END);
      }
      break;
    case HOOK:
      Serial.println("Executing hook...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_END);
      } else {
        servoMove(POS_END);
        servoMove(POS_START);
      }
      reverseSide = !reverseSide;
      break;
    case HALF_HOOK_AND_HOOK:
      Serial.println("Executing half hook and hook...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START_PLUS);
        servoMove(POS_END);
      } else {
        servoMove(POS_END);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END_MINUS);
        servoMove(POS_START);
      }
      reverseSide = !reverseSide;
      break;
    /*
    case HOOK_AND_HALF_HOOK:
      Serial.println("Executing hook and half hook...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_END_MINUS);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END);
      } else {
        servoMove(POS_END);
        servoMove(POS_START_PLUS);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START);
      }
      reverseSide = !reverseSide;
      break;
    case HALF_HOOK_X2:
      Serial.println("Executing double half hook...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START_PLUS);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_START);
      } else {
        servoMove(POS_START);
        servoMove(POS_END);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END_MINUS);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_END);
      }
      break;
    case HOOK_X3:
      Serial.println("Executing triple hook...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_END_MINUS);
        servoMove(POS_START_PLUS);
        servoMove(POS_END);
      } else {
        servoMove(POS_END);
        servoMove(POS_START_PLUS);
        servoMove(POS_END_MINUS);
        servoMove(POS_START);
      }
      reverseSide = !reverseSide;
      break;
    case HOOK_X2:
      Serial.println("Executing double hook...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_END_MINUS);
        servoMove(POS_START);
      } else {
        servoMove(POS_END);
        servoMove(POS_START_PLUS);
        servoMove(POS_END);
      }
      break;
    case FEINT_HOOK:
      Serial.println("Executing feint...");
      if(!reverseSide) {
        servoMove(POS_START);
        servoMove(POS_MIDDLE_MINUS);
        servoMove(POS_START);
      } else {
        servoMove(POS_END);
        servoMove(POS_MIDDLE_PLUS);
        servoMove(POS_END);
      }
      break;
    */
    default:
      Serial.println("Unknown punch type.");
  }
}


void handleStart() {
  if (server.hasArg("mode"))   currentMode      = server.arg("mode");
  if (server.hasArg("action")) currentAction    = server.arg("action");
  if (server.hasArg("pause"))  currentPauseMax  = server.arg("pause").toInt();
  isRunning = true;
  server.send(200, "text/plain", "Started");
}

void handleStop() {
  isRunning = false;
  server.send(200, "text/plain", "Stopped");
}

void handleSetServo() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    angle = constrain(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    int pulse = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo.write(pulse);
    currentPosition = angle;
  }
  server.send(200, "text/plain", "OK");
}



void setup() {
  Serial.begin(115200);
  accessPointInit();
  servoInit();

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", index_html);
  });
  server.on("/start", HTTP_GET, handleStart);
  server.on("/stop", HTTP_GET, handleStop);
  server.on("/setServo", HTTP_GET, handleSetServo);
  server.onNotFound([]() {
    server.send_P(200, "text/html", index_html);
  });
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  static unsigned long lastActionTime = 0;
  if (isRunning && millis() - lastActionTime > (unsigned long)currentPause) {
    lastActionTime = millis();
    if (currentMode == "straight") {
      if (currentAction == "random") {
        // random straight action by level
        int actionIdx = random(0, 2); // 0:STRAIGHT, 1:STRAIGHT_X2 //, 2:STRAIGHT_X3, 3:FEINT
        executeStraightAction(actionIdx);
      } else if (currentAction == "STRAIGHT") {
        executeStraightAction(STRAIGHT);
      } else if (currentAction == "STRAIGHT_X2") {
        executeStraightAction(STRAIGHT_X2);
      } else if (currentAction == "STRAIGHT_X3") {
        executeStraightAction(STRAIGHT_X3);
      } else if (currentAction == "FEINT") {
        executeStraightAction(FEINT_STRAIGHT);
      }
    } else if (currentMode == "hook") {
      if (currentAction == "random") {
        // random hook action by level
      int actionIdx = random(0, 3); //0:HALF_HOOK,1:HOOK,2:HALF_HOOK_AND_HOOK //,3:HOOK_AND_HALF_HOOK,4:HALF_HOOK_X2,5:HOOK_X3,6:HOOK_X2,7:FEINT_HOOK
      executeHookAction(actionIdx);
      } else if (currentAction == "HALF_HOOK") {
        executeHookAction(HALF_HOOK);
      } else if (currentAction == "HOOK") {
        executeHookAction(HOOK);
      } else if (currentAction == "HALF_HOOK_AND_HOOK") {
        executeHookAction(HALF_HOOK_AND_HOOK);
      } else if (currentAction == "HOOK_AND_HALF_HOOK") {
        executeHookAction(HOOK_AND_HALF_HOOK);
      } else if (currentAction == "HALF_HOOK_X2") {
        executeHookAction(HALF_HOOK_X2);
      } else if (currentAction == "HOOK_X3") {
        executeHookAction(HOOK_X3);
      } else if (currentAction == "HOOK_X2") {
        executeHookAction(HOOK_X2);
      } else if (currentAction == "FEINT") {
        executeHookAction(FEINT_HOOK);
      }
    }
    currentPause = random(1000, currentPauseMax);
  }
}
