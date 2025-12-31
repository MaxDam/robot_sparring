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
#define SERVO_MIN_PULSE_WIDTH 125 // this is the 'minimum' pulse length count (out of 4096)
#define SERVO_MAX_PULSE_WIDTH 575 // this is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQUENCY 60
//#define SERVO_MIN_PULSE_WIDTH 600
//#define SERVO_MAX_PULSE_WIDTH 2600
//#define SERVO_FREQUENCY 50

//joint
#define LEFT_STRAIGHT		0
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

//time calibration
#define SHOT_DURATION   350
#define SHOT_PAUSE      1000

//right stight calibration
#define RIGHT_STRIGHT_START_DEGREE 20
#define RIGHT_STRIGHT_END_DEGREE   95

//right hook calibration
#define RIGHT_HOOK_START_DEGREE    35
#define RIGHT_HOOK_END_DEGREE      105

//left straight calibration
#define LEFT_STRIGHT_START_DEGREE  15
#define LEFT_STRIGHT_END_DEGREE    85

//left hook calibration
#define LEFT_HOOK_START_DEGREE     35
#define LEFT_HOOK_END_DEGREE       95

//Servo wait
int pauseMax   = 3000; //not used
int servoDelay = 500;  //not used
int servoSpeed = 140;  //not used

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
  <h1>ROBOT SPARRING</h1>
  <div class='tabs'>
    <button class='tab-btn active' onclick='showTab(0)'>Actions</button>
    <button class='tab-btn' onclick='showTab(1)'>Config</button>
  </div>
  <div id='tab0' class='tab-content'>
    <label for='levelSelect'>Level:</label>
    <select id='levelSelect'>
      	<option value='2'>Easy</option>
		<option value='3'>Medium</option>
		<option value='4'>Pro</option>
    </select>
    <br>
	<label for='speedSelect'>Speed:</label>
	<select id='speedSelect'>
	<option value='0'>Slow</option>
	<option value='1'>Fast</option>
	<option value='2'>Very fast</option>
	</select>
    <br>
    <button id='startBtn'>Start</button>
    <button id='stopBtn'>Stop</button>    
  </div>
  <div id='tab1' class='tab-content' style='display:none;'>
    <div style='margin-top:20px;'>
      <label for='pauseRange'>Random pause (ms):</label>
      <input type='range' id='pauseRange' min='1000' max='10000' value='3000' />
      <span id='pauseValue' class='slider-value'>3000</span>
    </div>
    <div style='margin-top:30px;'>
      <label for='delaySlider'>Servo delay (ms):</label>
      <input type='range' id='delaySlider' min='10' max='5000' value='500' oninput="setDelay(this.value)"/>
      <span id='delayValue' class='slider-value'>500</span>
    </div>
     <div style='margin-top:30px;'>
      <label for='speedSlider'>Servo speed (ms):</label>
      <input type='range' id='speedSlider' min='100' max='500' value='140' oninput="setSpeed(this.value)"/>
      <span id='speedValue' class='slider-value'>140</span>
    </div>
  </div>
  <script>
    function showTab(idx) {
      document.querySelectorAll('.tab-btn').forEach((btn,i)=>btn.classList.toggle('active',i===idx));
      document.getElementById('tab0').style.display = idx===0?'block':'none';
      document.getElementById('tab1').style.display = idx===1?'block':'none';
    };
    document.getElementById('pauseRange').addEventListener('input', function() {
      document.getElementById('pauseValue').textContent = this.value;
    });
    function setSpeed(val) {
      document.getElementById("speedValue").innerText = val;
      fetch('/setSpeed?ms=' + val);
    };
    function setDelay(val) {
      document.getElementById("delayValue").innerText = val;
      fetch('/setDelay?ms=' + val);
    };
    var running = false;
    document.getElementById('startBtn').onclick = function() {
      if(running) return;
      running = true;
      var level = document.getElementById('levelSelect').value;
      var speed = document.getElementById('speedSelect').value;
      var pause = parseInt(document.getElementById('pauseRange').value);
      fetch('/start?level=' + level + '&speed=' + speed + '&pause=' + pause);
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
   Serial.print("Angle: ");Serial.print(ang);
   Serial.print(" pulse: ");Serial.println(pulse);
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

// not used
// Function to wait for servo movement to complete
void servoWait(int posStart, int posEnd) {
  if(servoDelay == 0) {
    servoDelay = ceil(abs(posEnd - posStart) * servoSpeed / 60);
  }
  if(servoDelay > 0) {
    delay(servoDelay);
  }
}

//basic movements

void startPosition() {
	pwm.setPWM(RIGHT_STRAIGHT, 0, angleToPulse(RIGHT_STRIGHT_START_DEGREE));
	pwm.setPWM(LEFT_STRAIGHT, 0, angleToPulse(LEFT_STRIGHT_START_DEGREE));
	pwm.setPWM(RIGHT_HOOK, 0, angleToPulse(RIGHT_HOOK_START_DEGREE));
	pwm.setPWM(LEFT_HOOK, 0, angleToPulse(LEFT_HOOK_START_DEGREE));
}

//executes the shot based on speed
void shotWithSpeed(int shot, int startDegree, int endDegree) {
  //set step based on the type of shot
  unsigned int step = 3;
  switch(shot) {
    case LEFT_STRAIGHT:
      //step = 4;
      step = 5;
      break;
    case LEFT_HOOK:
      step = 5;
      break;
    case RIGHT_HOOK:
      step = 5;
      break;
    case RIGHT_STRAIGHT:
      //step = 4;
      step = 5;
      break;
  }

  //throw the shot based on the speed
  switch(speed) {
      case VERYFAST:
        pwm.setPWM(shot, 0, angleToPulse(endDegree));
        delay(SHOT_DURATION);
        pwm.setPWM(shot, 0, angleToPulse(startDegree));
        break;
      case FAST:
        for (int angle = startDegree; angle <= endDegree; angle+=step) {
          pwm.setPWM(shot, 0, angleToPulse(angle));
          delay(30);
        }
        //delay(SHOT_DURATION);
        pwm.setPWM(shot, 0, angleToPulse(startDegree));
        break;
      case SLOW:
        for (int angle = startDegree; angle <= endDegree; angle+=step) {
          pwm.setPWM(shot, 0, angleToPulse(angle));
          delay(40);
        }
        //delay(SHOT_DURATION);
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
		delay(SHOT_DURATION);
		straightLeft();
	} else {
		straightRight();
		delay(SHOT_DURATION);
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
		delay(SHOT_DURATION);
		hookLeft();
	} else {
		hookRight();
		delay(SHOT_DURATION);
		hookRight();
	}
}

void shot_3_4(bool southpaw) {
	if(!southpaw) {
		hookLeft();
		delay(SHOT_DURATION);
		hookRight();
	} else {
		hookRight();
		delay(SHOT_DURATION);
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
		delay(SHOT_DURATION);
		straightRight();
	} else {
		straightRight();
		straightLeft();
		delay(SHOT_DURATION);
		straightLeft();
	}
}

void shot_1_1_2(bool southpaw) {
	if(!southpaw) {
		straightLeft();
		delay(SHOT_DURATION);
		straightLeft();
		straightRight();
	} else {
		straightRight();
		delay(SHOT_DURATION);
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
		delay(SHOT_DURATION);
		hookLeft();
	} else {
		straightRight();
		hookLeft();
		delay(SHOT_DURATION);
		hookRight();
	}
}

void shot_3_4_2(bool southpaw) {
	if(!southpaw) {
		hookLeft();
		delay(SHOT_DURATION);
		hookRight();
		straightRight();
	} else {
		hookRight();
		delay(SHOT_DURATION);
		hookLeft();
		straightLeft();
	}
}

void shot_3_4_3(bool southpaw) {
	if(!southpaw) {
		hookLeft();
		delay(SHOT_DURATION);
		hookRight();
		delay(SHOT_DURATION);
		hookLeft();
	} else {
		hookRight();
		delay(SHOT_DURATION);
		hookLeft();
		delay(SHOT_DURATION);
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
		delay(SHOT_DURATION);
		hookRight();
	} else {
		straightRight();
		straightLeft();
		hookRight();
		delay(SHOT_DURATION);
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
		delay(SHOT_DURATION);
		hookRight();
		straightRight();
		straightLeft();
	} else {
		hookRight();
		delay(SHOT_DURATION);
		hookLeft();
		straightLeft();
		straightRight();
	}
}

void shot_3_4_3_2(bool southpaw) {
	if(!southpaw) {
		hookLeft();
		delay(SHOT_DURATION);
		hookRight();
		delay(SHOT_DURATION);
		hookLeft();
		straightRight();
	} else {
		hookRight();
		delay(SHOT_DURATION);
		hookLeft();
		delay(SHOT_DURATION);
		hookRight();
		straightLeft();
	}
}

void shot_3_4_3_3(bool southpaw) {
	if(!southpaw) {
		hookLeft();
		delay(SHOT_DURATION);
		hookRight();
		delay(SHOT_DURATION);
		hookLeft();
		delay(SHOT_DURATION);
		hookLeft();
	} else {
		hookRight();
		delay(SHOT_DURATION);
		hookLeft();
		delay(SHOT_DURATION);
		hookRight();
		delay(SHOT_DURATION);
		hookRight();
	}
}

void shot_1_2_1_1(bool southpaw) {
	if(!southpaw) {
		straightLeft();
		straightRight();
		straightLeft();
		delay(SHOT_DURATION);
		straightLeft();
	} else {
		straightRight();
		straightLeft();
		straightRight();
		delay(SHOT_DURATION);
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
        int timeProbs[4] = {19, 27, 27, 27};
        waitingTimeMult = getRandomActionFromProbability(timeProbs, 4);
        return SHOT_DURATION + waitingTimeMult * 1000;
      }
      case MEDIUM: {
        int timeProbs[3] = {20, 40, 40};
        waitingTimeMult = getRandomActionFromProbability(timeProbs, 3);
        return SHOT_DURATION + waitingTimeMult * 1000;
      }
      case PRO: {
        int timeProbs[3] = {30, 35, 35};
        waitingTimeMult = getRandomActionFromProbability(timeProbs, 3);
        return SHOT_DURATION + waitingTimeMult * 1000;
      }
      default:
        return 100;
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
			delay(SHOT_DURATION);
			straightRight();
			delay(SHOT_DURATION);
			straightLeft();
			delay(SHOT_DURATION);
			hookRight();
			delay(SHOT_DURATION);
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
  if (server.hasArg("level")) level    = server.arg("level").toInt();
  if (server.hasArg("speed")) speed    = server.arg("speed").toInt();
  if (server.hasArg("pause")) pauseMax = server.arg("pause").toInt();
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
    servoDelay = ms;
  }
  server.send(200, "text/plain", "OK");
}

// Function to handle speed setting
void handleSetSpeed() {
  if (server.hasArg("ms")) {
    int ms = server.arg("ms").toInt();
    servoDelay = 0;
    servoSpeed = ms;
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
	server.on("/setSpeed", HTTP_GET, handleSetSpeed);
	server.on("/setDelay", HTTP_GET, handleSetDelay);
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
