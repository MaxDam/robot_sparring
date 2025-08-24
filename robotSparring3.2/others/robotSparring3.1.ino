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
 ****************************************************************************************************************************************************/

 
#include "WiFi.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <math.h>

// WiFi and AP
const char* ssid     = "Robot-Sparring-AP";
const char* password = "12345678";
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

//servo driver calibration
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
//#define SERVO_MIN_PULSE_WIDTH 102
//#define SERVO_MAX_PULSE_WIDTH 575
//#define SERVO_FREQUENCY 50
#define SERVO_MIN_PULSE_WIDTH 123
#define SERVO_MAX_PULSE_WIDTH 491
#define SERVO_FREQUENCY 40


//joint
#define LEFT_STRAIGHT	  0
#define LEFT_HOOK       1
#define RIGHT_HOOK	    14
#define RIGHT_STRAIGHT  15


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

//other properties
unsigned long shotCount = 0;
bool southpaw = true;


//right stight calibration
//#define RIGHT_STRIGHT_START_DEGREE 35
//#define RIGHT_STRIGHT_END_DEGREE   95
#define RIGHT_STRIGHT_START_DEGREE 25
#define RIGHT_STRIGHT_END_DEGREE   80

//left straight calibration
//#define LEFT_STRIGHT_START_DEGREE  35
//#define LEFT_STRIGHT_END_DEGREE    95
#define LEFT_STRIGHT_START_DEGREE  25
#define LEFT_STRIGHT_END_DEGREE    80


//right hook calibration
//#define RIGHT_HOOK_START_DEGREE    35
//#define RIGHT_HOOK_END_DEGREE      95
#define RIGHT_HOOK_START_DEGREE    25
#define RIGHT_HOOK_END_DEGREE      75

//left hook calibration
//#define LEFT_HOOK_START_DEGREE     35
//#define LEFT_HOOK_END_DEGREE       95
#define LEFT_HOOK_START_DEGREE     25
#define LEFT_HOOK_END_DEGREE       75


//Servo wait
int pauseMax     = 1000;
int shotDuration = 310;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="it">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Robot Sparring</title>
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
  <h1>ROBOT SPARRING</h1>

  <div class="tabs">
    <button class="tab-btn active" onclick="showTab(0)">Actions</button>
    <button class="tab-btn" onclick="showTab(1)">Config</button>
    <button class="tab-btn" onclick="showTab(2)">Manual</button>
  </div>

  <!-- TAB 0: ACTIONS -->
  <div id="tab0" class="tab-content">
    <label for="levelSelect">Level:</label>
    <select id="levelSelect">
      <option value="2">Easy</option>
      <option value="3">Medium</option>
      <option value="4">Pro</option>
    </select>
    <br>
    <label for="speedSelect">Speed:</label>
    <select id="speedSelect">
      <option value="0">Slow</option>
      <option value="1">Fast</option>
      <option value="2">Very fast</option>
    </select>
    <br>
    <button id="startBtn">Start</button>
    <button id="stopBtn">Stop</button>
  </div>

  <!-- TAB 1: CONFIG -->
  <div id="tab1" class="tab-content" style="display:none;">
    <div style="margin-top:20px;">
      <label for="pauseRange">Pause max (ms):</label>
      <input type="range" id="pauseRange" min="300" max="3000" value="1000" oninput="setPause(this.value)"/>
      <span id="pauseValue" class="slider-value">1000</span>
    </div>
    <div style="margin-top:30px;">
      <label for="delaySlider">Servo delay (ms):</label>
      <input type="range" id="delaySlider" min="200" max="600" value="310" oninput="setDelay(this.value)"/>
      <span id="delayValue" class="slider-value">310</span>
    </div>
  </div>

  <!-- TAB 2: MANUAL -->
  <div id="tab2" class="tab-content" style="display:none;">
    <div class="manual-wrap">
      <div class="robot-stage" style="background:url('data:image/png;base64,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') center/contain no-repeat; width:400px; height:246px;">
        <div class="hotspot hs-left"  data-arm="1" aria-label="Left hook"      onclick="shot('LEFT_HOOK')"></div>
    		<div class="hotspot hs-right" data-arm="2" aria-label="Right hook"     onclick="shot('RIGHT_HOOK')"></div>
    		<div class="hotspot hs-ul"    data-arm="3" aria-label="Left straight"  onclick="shot('LEFT_STRAIGHT')"></div>
    		<div class="hotspot hs-ur"    data-arm="4" aria-label="Right straight" onclick="shot('RIGHT_STRAIGHT')"></div>
      </div>
    </div>
  </div>

  <script>
    function showTab(idx) {
      document.querySelectorAll('.tab-btn').forEach((btn,i)=>btn.classList.toggle('active', i===idx));
      document.getElementById('tab0').style.display = idx===0?'block':'none';
      document.getElementById('tab1').style.display = idx===1?'block':'none';
      document.getElementById('tab2').style.display = idx===2?'block':'none';
    }
    function setPause(val) {
      document.getElementById("pauseValue").innerText = val;
      fetch('/setPause?ms=' + val);
    }
    function setDelay(val) {
      document.getElementById("delayValue").innerText = val;
      fetch('/setDelay?ms=' + val);
    }
	function shot(arm) {
	  var speed = document.getElementById('speedSelect').value;
	  fetch('/shot?arm=' + arm + '&speed=' + speed); 
	}
	
    var running = false;
    document.getElementById('startBtn').onclick = function() {
      if(running) return;
      running = true;
      var level = document.getElementById('levelSelect').value;
      var speed = document.getElementById('speedSelect').value;
      fetch('/start?level=' + level + '&speed=' + speed);
    };
    document.getElementById('stopBtn').onclick = function() {
      running = false;
      fetch('/stop');
    };
  </script>
</body>
</html>
)rawliteral";

//drive servo (angle -> pulse)
int angleToPulse(int ang){
   int pulse = map(ang, 0, 180, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH); // map angle of 0 to 180 to Servo min and Servo max 
   Serial.print("Angle: ");
   Serial.print(ang);
   Serial.print(" pulse: ");
   Serial.println(pulse);
   return pulse;
}

//init servo
void servoInit() {
	pwm.begin();
	pwm.setOscillatorFrequency(27000000);
	pwm.setPWMFreq(SERVO_FREQUENCY);
	delay(10);
}

// Function to initialize the access point
void accessPointInit() {
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("Access Point avviato. IP:");
  Serial.println(myIP);
  dnsServer.start(DNS_PORT, "*", myIP);
}

//basic movements

void startPosition() {
	pwm.setPWM(RIGHT_STRAIGHT, 0, angleToPulse(RIGHT_STRIGHT_START_DEGREE));
	pwm.setPWM(LEFT_STRAIGHT,  0, angleToPulse(LEFT_STRIGHT_START_DEGREE));
	pwm.setPWM(RIGHT_HOOK,     0, angleToPulse(RIGHT_HOOK_START_DEGREE));
	pwm.setPWM(LEFT_HOOK,      0, angleToPulse(LEFT_HOOK_START_DEGREE));
}


//executes the shot based on speed
void shotWithSpeed(int shot, int startDegree, int endDegree) {
  //set step based on the type of shot
  unsigned int step = 3;
  switch(shot) {
    case LEFT_STRAIGHT:
      step = 5;
      break;
    case LEFT_HOOK:
      step = 5;
      break;
    case RIGHT_HOOK:
      step = 5;
      break;
    case RIGHT_STRAIGHT:
      step = 5;
      break;
  }

  
  //throw the shot based on the speed
  switch(speed) {
      case VERYFAST:
        pwm.setPWM(shot, 0, angleToPulse(endDegree));
        delay(shotDuration);
        pwm.setPWM(shot, 0, angleToPulse(startDegree));
        break;
      case FAST:
        for (int angle = startDegree; angle <= endDegree; angle+=step) {
          pwm.setPWM(shot, 0, angleToPulse(angle));
          delay(30);
        }
        pwm.setPWM(shot, 0, angleToPulse(startDegree));
        break;
      case SLOW:
        for (int angle = startDegree; angle <= endDegree; angle+=step) {
          pwm.setPWM(shot, 0, angleToPulse(angle));
          delay(40);
        }
        pwm.setPWM(shot, 0, angleToPulse(startDegree));
        break;
  }
}

void straightRight() {
  shotWithSpeed(RIGHT_STRAIGHT, RIGHT_STRIGHT_START_DEGREE, RIGHT_STRIGHT_END_DEGREE);
}

void straightLeft() {
  shotWithSpeed(LEFT_STRAIGHT, LEFT_STRIGHT_START_DEGREE, LEFT_STRIGHT_END_DEGREE);
}

void hookRight() {
  shotWithSpeed(RIGHT_HOOK, RIGHT_HOOK_START_DEGREE, RIGHT_HOOK_END_DEGREE);
}

void hookLeft() {
  shotWithSpeed(LEFT_HOOK, LEFT_HOOK_START_DEGREE, LEFT_HOOK_END_DEGREE);
}

//shot combinations

void shot_1(bool southpaw) {
	if(!southpaw) {
		straightLeft();
	} else {
		straightRight();
	}
}

void shot_3(bool southpaw) {
	if(!southpaw) {
		hookLeft();
	} else {
		hookRight();
	}
}

void shot_1_2(bool southpaw) {
	if(!southpaw) {
		straightLeft();
		straightRight();
	} else {
		straightRight();
		straightLeft();
	}
}

void shot_2_3(bool southpaw) {
	if(!southpaw) {
		straightRight();
		hookLeft();
	} else {
		straightLeft();
		hookRight();
	}
}

void shot_1_1(bool southpaw) {
	if(!southpaw) {
		straightLeft();
		delay(shotDuration);
		straightLeft();
	} else {
		straightRight();
		delay(shotDuration);
		straightRight();
	}
}

void shot_3_2(bool southpaw) {
	if(!southpaw) {
		hookLeft();
		straightRight();
	} else {
		hookRight();
		straightLeft();
	}
}

void shot_1_3(bool southpaw) {
	if(!southpaw) {
		straightLeft();
		hookLeft();
	} else {
		straightRight();
		hookRight();
	}
}

void shot_3_3(bool southpaw) {
	if(!southpaw) {
		hookLeft();
		delay(shotDuration);
		hookLeft();
	} else {
		hookRight();
		delay(shotDuration);
		hookRight();
	}
}

void shot_3_4(bool southpaw) {
	if(!southpaw) {
		hookLeft();
		delay(shotDuration);
		hookRight();
	} else {
		hookRight();
		delay(shotDuration);
		hookLeft();
	}
}

void shot_1_2_3(bool southpaw) {
	if(!southpaw) {
		straightLeft();
		straightRight();
		hookLeft();
	} else {
		straightRight();
		straightLeft();
		hookRight();
	}
}

void shot_2_3_2(bool southpaw) {
	if(!southpaw) {
		straightRight();
		hookLeft();
		straightRight();
	} else {
		straightLeft();
		hookRight();
		straightLeft();
	}
}

void shot_1_3_1(bool southpaw) {
	if(!southpaw) {
		straightLeft();
		hookLeft();
		straightLeft();
	} else {
		straightRight();
		hookRight();
		straightRight();
	}
}

void shot_1_2_2(bool southpaw) {
	if(!southpaw) {
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

void shot_1_1_2(bool southpaw) {
	if(!southpaw) {
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

void shot_1_2_1(bool southpaw) {
	if(!southpaw) {
		straightLeft();
		straightRight();
		straightLeft();
	} else {
		straightRight();
		straightLeft();
		straightRight();
	}
}

void shot_1_4_3(bool southpaw) {
	if(!southpaw) {
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

void shot_3_4_2(bool southpaw) {
	if(!southpaw) {
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

void shot_3_4_3(bool southpaw) {
	if(!southpaw) {
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

void shot_3_1_3(bool southpaw) {
	if(!southpaw) {
		hookLeft();
		straightLeft();
		hookLeft();
	} else {
		hookRight();
		straightRight();
		hookRight();
	}
}

void shot_1_2_3_4(bool southpaw) {
	if(!southpaw) {
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

void shot_1_2_1_2(bool southpaw) {
	if(!southpaw) {
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

void shot_1_2_3_2(bool southpaw) {
	if(!southpaw) {
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

void shot_3_4_2_1(bool southpaw) {
	if(!southpaw) {
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

void shot_3_4_3_2(bool southpaw) {
	if(!southpaw) {
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

void shot_3_4_3_3(bool southpaw) {
	if(!southpaw) {
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

void shot_1_2_1_1(bool southpaw) {
	if(!southpaw) {
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

//get wait time (level based)
int getRandomWaitTime() {
     
	unsigned int waitingTimeMult = 0;
    
    switch(level) {
      case EASY: {
        int timeProbs[4] = {1, 33, 33, 33};
        waitingTimeMult = getRandomActionFromProbability(timeProbs, 4);
        return shotDuration + waitingTimeMult * pauseMax;
      }
      case MEDIUM: {
        int timeProbs[3] = {4, 48, 48};
        waitingTimeMult = getRandomActionFromProbability(timeProbs, 3);
        return shotDuration + waitingTimeMult * pauseMax;
      }
      case PRO: {
        int timeProbs[3] = {8, 46, 46};
        waitingTimeMult = getRandomActionFromProbability(timeProbs, 3);
        return shotDuration + waitingTimeMult * pauseMax;
      }
      default:
        return shotDuration;
  }
}

//wait the shot pause time
void waitShotPauseTime() {
  int waitTime = getRandomWaitTime();
  for(int i = 0; i < 10; i++) {
    delay(waitTime/10);
  }
}

//get action (random probability) level based
int getActionFromState() {
	int action = 0;
	switch(level) {
		case START: {    
			startPosition();
			delay(shotDuration);
			straightRight();
			delay(shotDuration);
			straightLeft();
			delay(shotDuration);
			hookRight();
			delay(shotDuration);
			hookLeft();
			
			level = STOP;
			action = 0;
			break;
		}
		case STOP: {
			action = 0;
			break;
		}
    case EASY: {
			int shotNumberProbs[2] = {70, 30};
			action = getRandomActionFromProbability(shotNumberProbs, 2) + 1;
			break;
		}
		case MEDIUM: {
			int shotNumberProbs[3] = {30, 30, 40};
			action = getRandomActionFromProbability(shotNumberProbs, 3) + 1;
			break;
		}
		case PRO: {
			int shotNumberProbs[4] = {10, 20, 35, 35};
			action = getRandomActionFromProbability(shotNumberProbs, 4) + 1;
			break;
		}
	}
	return action;
}

//change shot side based on probability
void checkChangeSide() {
	int changeSideProbs[2] = {80, 20};
	if(getRandomActionFromProbability(changeSideProbs, 2) == 0) {
		southpaw = !southpaw;
	}
}

//execute the action (random probability based)
void executeAction(int action)  {
  switch(action) {
    case 0: { //no action
      startPosition();
      break;
    }
	  case 1: { //single shot
  		int singleShotProbs[2] = {50, 50};
  		switch(getRandomActionFromProbability(singleShotProbs, 2)) {
  			case 0: shot_1(southpaw); break;
  			case 1: shot_3(southpaw); break;
  		}
		  break;
	  }
	  case 2: { //double shot
  		int doubleShotProbs[7] = {15, 14, 14, 14, 15, 14, 14};
  		switch(getRandomActionFromProbability(doubleShotProbs, 7)) {
  			case 0: shot_1_2(southpaw);  break;
  			case 1: shot_3_3(southpaw);  break;
  			case 2: shot_1_1(southpaw);  break;
  			case 3: shot_3_2(southpaw);  break;
  			case 4: shot_1_3(southpaw);  break;
  			case 5: shot_2_3(southpaw);  break;
  			case 6: shot_3_4(southpaw);  break;
  		}
  		break;
	  }
	  case 3: { //triple shot
		  int tripleShotProbs[10] = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
  		switch(getRandomActionFromProbability(tripleShotProbs, 10)) {
  			case 0: shot_1_2_3(southpaw);  break;
  			case 1: shot_3_1_3(southpaw);  break;
  			case 2: shot_1_3_1(southpaw);  break;
  			case 3: shot_3_4_2(southpaw);  break;
  			case 4: shot_1_1_2(southpaw);  break;
  			case 5: shot_1_2_1(southpaw);  break;
  			case 6: shot_3_4_3(southpaw);  break;
  			case 7: shot_1_2_2(southpaw);  break;
  			case 8: shot_1_4_3(southpaw);  break;
  			case 9: shot_2_3_2(southpaw);  break;
  		}
  		break;
	  }
	  case 4: { //quadruple shot
  		int quadrupleShotProbs[7] = {15, 14, 14, 14, 15, 14, 14};
  		switch(getRandomActionFromProbability(quadrupleShotProbs, 7)) {
  			case 0: shot_1_2_3_4(southpaw);  break;
  			case 1: shot_3_4_3_3(southpaw);  break;
  			case 2: shot_1_2_3_2(southpaw);  break;
  			case 3: shot_3_4_2_1(southpaw);  break;
  			case 4: shot_1_2_1_2(southpaw);  break;
  			case 5: shot_3_4_3_2(southpaw);  break;
  			case 6: shot_1_2_1_1(southpaw);  break;
  		}
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
  if (server.hasArg("level")) level = server.arg("level").toInt();
  if (server.hasArg("speed")) speed = server.arg("speed").toInt();
  server.send(200, "text/plain", "Started");
}

// Function to handle stop request
void handleStop() {
  level = STOP;
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
void handleSetPause() {
  if (server.hasArg("ms")) {
    int ms = server.arg("ms").toInt();
    pauseMax = ms;
  }
  server.send(200, "text/plain", "OK");
}

// Function to handle manual single shot
void handleShot() {
  if (server.hasArg("arm")) {
    speed = server.arg("speed").toInt();
	String arm = server.arg("arm");
	
    if(arm == "LEFT_STRAIGHT") {
		straightLeft();
	}
	if(arm == "RIGHT_STRAIGHT") {
		straightRight();
	}
	if(arm == "LEFT_HOOK") {
		hookLeft();
	}
	if(arm == "RIGHT_HOOK") {
		hookRight();
	}
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
	server.on("/start",    HTTP_GET, handleStart);
	server.on("/stop",     HTTP_GET, handleStop);
	server.on("/setDelay", HTTP_GET, handleSetDelay);
	server.on("/setPause", HTTP_GET, handleSetPause);
	server.on("/shot",     HTTP_GET, handleShot);
	server.onNotFound([]() {
		server.send_P(200, "text/html", index_html);
	});
	server.begin();
}

void loop() {
	dnsServer.processNextRequest();
  server.handleClient();

	//random action level based
	int action = getActionFromState();
	checkChangeSide();
	executeAction(action);
  waitShotPauseTime();
}
