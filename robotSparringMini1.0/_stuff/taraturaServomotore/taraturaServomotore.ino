/*
Installare libreria ESP32Servo
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <DNSServer.h>

const char* ssid = "Servo-ESP32";
const char* password = "12345678";

WebServer server(80);
Servo myservo;
DNSServer dnsServer;
const byte DNS_PORT = 53;

const int servoPin = 17;
int pwmValue = 1500; // neutro

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>Servo 360° Taratura</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body {
        font-family: sans-serif;
        text-align: center;
        margin-top: 50px;
        padding: 0 20px;
        font-size: 22px;
      }

      h1 {
        font-size: 32px;
        margin-bottom: 40px;
      }

      input[type=range] {
        width: 90%;
        height: 40px;
        -webkit-appearance: none;
        background: #ccc;
        border-radius: 10px;
        outline: none;
        margin: 30px 0;
      }

      input[type=range]::-webkit-slider-thumb {
        -webkit-appearance: none;
        width: 40px;
        height: 40px;
        background: #444;
        border-radius: 50%;
        cursor: pointer;
      }

      input[type=range]::-moz-range-thumb {
        width: 40px;
        height: 40px;
        background: #444;
        border: none;
        border-radius: 50%;
        cursor: pointer;
      }

      #pwmVal {
        font-size: 28px;
        font-weight: bold;
        color: #333;
      }

      .button-container {
        display: flex;
        gap: 10px;
        margin: 20px;
      }
  
      button {
        padding: 10px 20px;
        font-size: 16px;
        cursor: pointer;
      }
    </style>
  </head>
  <body>

    <h1>Azioni 360</h1>
    <div class="button-container">
      <button onclick="sendAction(document.getElementById('velDestraVeloce').value)">RF</button>
      <button onclick="sendAction(document.getElementById('velDestraLenta').value)">RS</button>
      <button onclick="sendAction(1500)">Stop</button>
      <button onclick="sendAction(document.getElementById('velSinistraLenta').value)">LS</button>
      <button onclick="sendAction(document.getElementById('velSinistraVeloce').value)">LF</button>
    </div>

    <h1>Parametri di Velocità 360</h1>
     <div>
        <label for="velDestraVeloce">Destra Veloce</label><br>
        <input type="number" id="velDestraVeloce" value="2500" min="500" max="2500">
      </div>
     <div style="display: flex; justify-content: center; gap: 20px; flex-wrap: wrap;">
      <div>
        <label for="velDestraLenta">Destra Lenta</label><br>
        <input type="number" id="velDestraLenta" value="1800" min="500" max="2500">
      </div>
     <div>
        <label for="velSinistraLenta">Sinistra Lenta</label><br>
        <input type="number" id="velSinistraLenta" value="1200" min="500" max="2500">
      </div>
      <div>
        <label for="velSinistraVeloce">Sinistra Veloce</label><br>
        <input type="number" id="velSinistraVeloce" value="600" min="500" max="2500">
      </div>
    </div>

    <h1>Velocità manuale 360</h1>
    <p>PWM: <span id="pwmVal">1500</span> µs</p>
    <input type="range" min="500" max="2500" value="1500" id="slider" oninput="updatePWM(this.value)">

    <h1>Angolo 180</h1>
    <p>PWM: <span id="angle180Val">90</span></p>
    <input type="range" min="0" max="180" value="90" id="slider" oninput="updatePulse180(this.value)">

    <h1>Angolo 270</h1>
    <p>PWM: <span id="angle270Val">135</span></p>
    <input type="range" min="0" max="270" value="135" id="slider" oninput="updatePulse270(this.value)">

    <br><br><br>
 
    <script>
      function sendAction(val) {
        fetch("/setAction?value=" + val);
      };
      function updatePWM(val) {
        document.getElementById("pwmVal").innerText = val;
        fetch("/setPWM?value=" + val);
      };
      function updatePulse180(val) {
        document.getElementById("angle180Val").innerText = val;
        fetch("/setPulse180?value=" + val);
      };
      function updatePulse270(val) {
        document.getElementById("angle270Val").innerText = val;
        fetch("/setPulse270?value=" + val);
      };
      
    </script>
  </body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("Access Point avviato. IP:");
  Serial.println(myIP);
  dnsServer.start(DNS_PORT, "*", myIP);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2500);

  myservo.writeMicroseconds(pwmValue);
  delay(500);
  myservo.writeMicroseconds(1200);
  delay(500);
  myservo.writeMicroseconds(1800);
  delay(1000);
  myservo.writeMicroseconds(1200);
  delay(500);
  myservo.writeMicroseconds(pwmValue);

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", htmlPage);
  });

  server.on("/setPWM", HTTP_GET, []() {
    if (server.hasArg("value")) {
      pwmValue = server.arg("value").toInt();
      pwmValue = constrain(pwmValue, 500, 2500);
      myservo.writeMicroseconds(pwmValue);
      Serial.print("PWM: ");
      Serial.println(pwmValue);
    }
    
    server.send(200, "text/plain", "OK");
  });

  server.on("/setAngle", HTTP_GET, []() {
    if (server.hasArg("value")) {
      int pos = server.arg("value").toInt();
      myservo.write(pos);
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/setPulse180", HTTP_GET, []() {
    if (server.hasArg("value")) {
      int pos = server.arg("value").toInt();
      int pulse = map(pos, 0, 180, 500, 2500);
      myservo.write(pulse);
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/setPulse270", HTTP_GET, []() {
    if (server.hasArg("value")) {
      int pos = server.arg("value").toInt();
      int pulse = map(pos, 0, 270, 500, 2500);
      myservo.write(pulse);
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/setAction", HTTP_GET, []() {
    if (server.hasArg("value")) {
      int pwmValue = server.arg("value").toInt();
      pwmValue = constrain(pwmValue, 500, 2500);
      myservo.writeMicroseconds(pwmValue);
    }
    server.send(200, "text/plain", "OK");
  });

  server.onNotFound([]() {
    server.send_P(200, "text/html", htmlPage);
  });

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
