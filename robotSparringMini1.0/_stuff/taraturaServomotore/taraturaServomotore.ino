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

const int servoPin = 18;
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
    </style>
  </head>
  <body>
    <h1>Taratura Servo 360°</h1>
    <p>PWM: <span id="pwmVal">1500</span> µs</p>
    <input type="range" min="500" max="2500" value="1500" id="slider" oninput="updatePWM(this.value)">
    
    <script>
      function updatePWM(val) {
        document.getElementById("pwmVal").innerText = val;
        fetch("/setPWM?value=" + val);
      }
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

  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2500);
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

  server.onNotFound([]() {
    server.send_P(200, "text/html", htmlPage);
  });

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
