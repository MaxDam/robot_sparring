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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//wifi
char* ssid = "xxx";
char* password = "xxx";
WiFiClient espClient;

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

//display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//state
#define STOP            0
#define START           1
#define EASY            2
#define MEDIUM          3
#define PRO             4

//vars
unsigned int state = START;
unsigned long shotCount = 0;
bool southpaw = true;

//time calibration
#define SHOT_DURATION   500
#define SHOT_PAUSE      1000

//right stight calibration
#define RIGHT_STRIGHT_START_DEGREE 20
#define RIGHT_STRIGHT_END_DEGREE   90

//right hook calibration
#define RIGHT_HOOK_START_DEGREE    35
#define RIGHT_HOOK_END_DEGREE      140

//left straight calibration
#define LEFT_STRIGHT_START_DEGREE  20
#define LEFT_STRIGHT_END_DEGREE    80

//left hook calibration
#define LEFT_HOOK_START_DEGREE     35
#define LEFT_HOOK_END_DEGREE       120

//drive servo (angle -> pulse)
int angleToPulse(int ang){
   int pulse = map(ang, 0, 180, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);// map angle of 0 to 180 to Servo min and Servo max 
   Serial.print("Angle: ");Serial.print(ang);
   Serial.print(" pulse: ");Serial.println(pulse);
   return pulse;
}

/*
int pulseWidth(int angle){
   int pulse_wide, analog_value;
   pulse_wide = map(angle, 0, 180, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
   analog_value = int(float(pulse_wide) / 1000000 * SERVO_FREQUENCY * 4096);
   Serial.print("Angle: ");Serial.print(angle);
   Serial.print(" pulse: ");Serial.println(pulse_wide);
   Serial.print(" analog value: ");Serial.println(analog_value);
   return analog_value;
}
*/

//init servo driver
void initServoDriver() {
	pwm.begin();
	pwm.setOscillatorFrequency(27000000);
	pwm.setPWMFreq(SERVO_FREQUENCY);
	delay(10);
}

//init display
void initDisplay() {
	// initialize OLED display with I2C address 0x3C
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
		Serial.println(F("failed to start SSD1306 OLED"));
		while (1);
	}

	delay(2000);        
	display.clearDisplay(); 

	display.setTextSize(2);         
  display.setTextColor(WHITE);    
	display.setCursor(0, 15);       
	display.println("Hello!");     
	display.display();  
  delay(1000);  

	//init buttons
	//pinMode (19, INPUT_PULLDOWN);
	pinMode (19, INPUT_PULLUP); //BACK
	pinMode (18, INPUT_PULLUP); //OK
	pinMode (17, INPUT_PULLUP); //UP
	pinMode (16, INPUT_PULLUP); //DOWN
}

//display string text on the center
void displayText(String text) {
	display.clearDisplay(); 
	display.setCursor(0, 15);
	display.println(text);     
	display.display(); 
}

//display
int selected = 0;
int entered = -1;

//check menu button status
void checkButtons() {
  int up     = digitalRead(16);
  int down    = digitalRead(17);
  int enter  = digitalRead(18);
  int back   = digitalRead(19);
  if (back == LOW || enter == LOW || up == LOW || down == LOW) {
    displayLevelMenu();
  }
}

//menu
void displayLevelMenu() {
  int down   = digitalRead(16);
  int up     = digitalRead(17);
  int enter  = digitalRead(18);
	int back 	 = digitalRead(19);
  
	if (up == LOW && down == LOW) {
	};
	if (up == LOW) {
    selected = selected - 1;
    if(selected < 0) selected = 0;
    delay(200);
	};
	if (down == LOW) {	
    selected = selected + 1;
    if(selected > 2) selected = 2;
    delay(200);
	};
	if (enter == LOW) {
		entered = selected;
	};
	if (back == LOW) {
		entered = -1;
		state = STOP;
	};
	const char *options[3] = {
		"EASY      ",
		"MEDIUM    ",
		"PRO       "
	};

	if(entered == -1) {
		display.clearDisplay();
		display.setTextSize(2);
		display.setTextColor(WHITE);
		display.setCursor(0, 5);
		for (int i = 0; i < 3; i++) {
			if (i == selected) {
				display.setTextColor(BLACK, WHITE);
        display.println(options[i]);
			} else if (i != selected) {
				display.setTextColor(WHITE);
				display.println(options[i]);
			}
		}
	}
	else if(entered == 0) {
		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(WHITE);
		display.setCursor(0, 10);
		display.println("Level:");
		display.setTextSize(2);
		display.println("EASY");
		display.setTextColor(WHITE);
		state = EASY;
	} 
	else if(entered == 1) {
		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(WHITE);
		display.setCursor(0, 10);
		display.println("Level:");
		display.setTextSize(2);
		display.println("MEDIUM");
		display.setTextColor(WHITE);
		state = MEDIUM;
	} 
	else if(entered == 2) {
		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(WHITE);
		display.setCursor(0, 10);
		display.println("Level:");
		display.setTextSize(2);
		display.println("PRO");
		state = PRO;
	}

	display.display();
}

//init WiFi
void initWiFi() {
	// Connect to the network
	WiFi.begin(ssid, password);
	Serial.print("Connecting to ");
	Serial.print(ssid); Serial.println(" ...");

	// Wait for the Wi-Fi to connect
	int i = 0;
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.print(++i); Serial.print(' ');
	}

	// Send the IP address of the ESP8266 to the computer
	Serial.println('\n');
	Serial.println("Connection established!");  
	Serial.print("IP address:\t");
	Serial.println(WiFi.localIP());
	Serial.println("");
	//display.write(WiFi.localIP());

	delay(1000);
}

//basic movements

void startPosition() {
	pwm.setPWM(RIGHT_STRAIGHT, 0, angleToPulse(RIGHT_STRIGHT_START_DEGREE));
	pwm.setPWM(LEFT_STRAIGHT, 0, angleToPulse(LEFT_STRIGHT_START_DEGREE));
	pwm.setPWM(RIGHT_HOOK, 0, angleToPulse(RIGHT_HOOK_START_DEGREE));
	pwm.setPWM(LEFT_HOOK, 0, angleToPulse(LEFT_HOOK_START_DEGREE));
  checkButtons();
}

void straightRight() {
	//displayText("straight right");
	pwm.setPWM(RIGHT_STRAIGHT, 0, angleToPulse(RIGHT_STRIGHT_END_DEGREE));
	delay(SHOT_DURATION);
	pwm.setPWM(RIGHT_STRAIGHT, 0, angleToPulse(RIGHT_STRIGHT_START_DEGREE));
  checkButtons();
}

void straightLeft() {
	//displayText("straight left");
	pwm.setPWM(LEFT_STRAIGHT, 0, angleToPulse(LEFT_STRIGHT_END_DEGREE));
	delay(SHOT_DURATION);
	pwm.setPWM(LEFT_STRAIGHT, 0, angleToPulse(LEFT_STRIGHT_START_DEGREE));
  checkButtons();
}

void hookRight() {
	//displayText("hook right");
	pwm.setPWM(RIGHT_HOOK, 0, angleToPulse(RIGHT_HOOK_END_DEGREE));
	delay(SHOT_DURATION);
	pwm.setPWM(RIGHT_HOOK, 0, angleToPulse(RIGHT_HOOK_START_DEGREE));
  checkButtons();
}

void hookLeft() {
	//displayText("hook left");
	pwm.setPWM(LEFT_HOOK, 0, angleToPulse(LEFT_HOOK_END_DEGREE));
	delay(SHOT_DURATION);
	pwm.setPWM(LEFT_HOOK, 0, angleToPulse(LEFT_HOOK_START_DEGREE));
  checkButtons();
}

//shot combinations

void shot_1(bool southpaw) {
	//displayText("one");
	if(!southpaw) {
		straightLeft();
	} else {
		straightRight();
	}
}

void shot_3(bool southpaw) {
	//displayText("three");
	if(!southpaw) {
		hookLeft();
	} else {
		hookRight();
	}
}

void shot_1_2(bool southpaw) {
	//displayText("one - two");
	if(!southpaw) {
		straightLeft();
		straightRight();
	} else {
		straightRight();
		straightLeft();
	}
}

void shot_2_3(bool southpaw) {
	//displayText("two - three");
	if(!southpaw) {
		straightRight();
		hookLeft();
	} else {
		straightLeft();
		hookRight();
	}
}

void shot_1_1(bool southpaw) {
	//displayText("jab - jab");
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
	//displayText("three - two");
	if(!southpaw) {
		hookLeft();
		straightRight();
	} else {
		hookRight();
		straightLeft();
	}
}

void shot_1_3(bool southpaw) {
	//displayText("one - three");
	if(!southpaw) {
		straightLeft();
		hookLeft();
	} else {
		straightRight();
		hookRight();
	}
}

void shot_3_3(bool southpaw) {
	//displayText("three - three");
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
	//displayText("three - three");
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
	//displayText("one - two - three");
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
	//displayText("two - three - two");
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
	//displayText("one - three - one");
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
	//displayText("one - two - two");
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
	//displayText("one - one - two");
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
	//displayText("one - two - one");
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
	//displayText("one - four - three");
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
	//displayText("three - four - two");
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
	//displayText("three - four - three");
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
	//displayText("three - one - three");
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
	//displayText("one - two - three - four");
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
	//displayText("one - two - one - two");
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
	//displayText("one - two - three - two");
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
	//displayText("three - four - two - one");
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
	//displayText("three - four - three - four");
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
	//displayText("three - four - three - three");
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
	//displayText("one - two - one - one");
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
    
    switch(state) {
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
  checkButtons();
  int waitTime = getRandomWaitTime();
  for(int i = 0; i < 10; i++) {
    delay(waitTime/10);
    checkButtons();
  }
}

//get action (random probability) state based
int getActionFromState() {
	int action = 0;
	switch(state) {
		case START: {
      displayText("Start...");
    
			startPosition();
			delay(SHOT_PAUSE);
			straightRight();
			delay(SHOT_PAUSE);
			straightLeft();
			delay(SHOT_PAUSE);
			hookRight();
			delay(SHOT_PAUSE);
			hookLeft();
			
			state = STOP;
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
		default: startPosition();
	}

	shotCount++;
}

void setup() {
	Serial.begin(115200);
	initServoDriver();
	initDisplay();
	//initWiFi();
}

void loop() {
	displayLevelMenu();

	//random action state based
	int action = getActionFromState();
	checkChangeSide();
	executeAction(action);

  waitShotPauseTime();
}
