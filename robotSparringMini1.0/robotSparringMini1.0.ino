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

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

const char* ssid = "Robot-Sparring-AP";
const char* password = "12345678";

WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>pannello controllo</title>
  <style>
    body {
      background-color: #2c2c2c;
      color: #e0e0e0;
      font-family: Arial, sans-serif;
      text-align: center;
      padding: 40px;
    }
    h1 {
      font-size: 2.8rem;
      color: #ffffff;
      font-weight: 600;
      margin-bottom: 35px;
      letter-spacing: 1px;
      text-transform: uppercase;
      text-shadow: 1px 2px 4px rgba(0, 0, 0, 0.6);
      border-bottom: 2px solid #444;
      display: inline-block;
      padding-bottom: 8px;
    }
    button {
      margin: 3px;
      font: inherit;
      background-color: #f0f0f0;
      border: 0;
      color: #242424;
      border-radius: 0.5em;
      font-size: 1.35rem;
      padding: 0.375em 1em;
      font-weight: 600;
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
    .checkbox-group {
      display: flex;
      justify-content: center;
      gap: 10px;
      margin: 40px 0 30px 0;
    }
    .customCheckBoxHolder { margin: 5px; display: flex; }
    .customCheckBox {
      padding: 2px 8px;
      background-color: rgba(0, 0, 0, 0.16);
      display: flex;
      align-items: center;
      justify-content: center;
      min-width: 55px;
      height: 32px;
      cursor: pointer;
    }
    .customCheckBox .inner {
      font-size: 18px;
      font-weight: 900;
      pointer-events: none;
    }
    .customCheckBoxInput { display: none; }
    .customCheckBoxInput:checked + .customCheckBoxWrapper .customCheckBox {
      background-color: #ffffff;
      color: #000;
    }
    .difficolta-wrapper {
      margin-top: 50px;
    }
    .difficolta-wrapper label {
      font-size: 1.2rem;
      margin-bottom: 10px;
      display: block;
    }
    input[type="range"] {
      width: 90%;
      margin-top: 20px;
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
  </style>
</head>
<body>
  <h1>ROBOT SPARRING</h1>
  <div>
    <button type="button" id="Start">Start</button>
    <button type="button" id="Stop">Stop</button>
  </div>

  <div class="checkbox-group">
    <span class="customCheckBoxHolder">
      <input type="radio" id="cCB1" name="scelta" class="customCheckBoxInput" checked>
      <label for="cCB1" class="customCheckBoxWrapper">
        <span class="customCheckBox"><span class="inner">Ganci</span></span>
      </label>
    </span>
    <span class="customCheckBoxHolder">
      <input type="radio" id="cCB2" name="scelta" class="customCheckBoxInput">
      <label for="cCB2" class="customCheckBoxWrapper">
        <span class="customCheckBox"><span class="inner">Diretti</span></span>
      </label>
    </span>
  </div>

  <div class="difficolta-wrapper">
    <label for="difficolta">Difficoltà: <span id="valoreDifficolta">3</span></label>
    <input type="range" id="difficolta" name="difficolta" min="1" max="4" value="3">
  </div>

  <script>
    document.addEventListener('DOMContentLoaded', function () {
      const stopButton = document.getElementById('Stop');
      const startButton = document.getElementById('Start');
      const ganci = document.getElementById('cCB1');
      const diretti = document.getElementById('cCB2');
      const slider = document.getElementById('difficolta');

      const valoreDifficolta = document.getElementById('valoreDifficolta');
      slider.addEventListener('input', function () {
        valoreDifficolta.textContent = slider.value;
      });

      function getTipoColpo() {
        return ganci.checked ? 'ganci' : diretti.checked ? 'diretti' : null;
      }

      function inviaAzione(azione) {
        const tipo = getTipoColpo();
        const difficolta = slider.value;

        if (!tipo) {
          alert("Seleziona 'Ganci' o 'Diretti' prima di continuare.");
          return;
        }

        fetch("/api/start", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ azione, tipo, difficolta })
        })
        .then(res => res.text())
        //.then(data => alert("ESP32 dice: " + data))
        .catch(err => alert("Errore: " + err));
      }

      startButton.addEventListener('click', function () {
        inviaAzione("start");
      });

      stopButton.addEventListener('click', function () {
        inviaAzione("stop");
      });
    });
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

  server.on("/api/start", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      Serial.println("Dati ricevuti:");
      Serial.println(body);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Nessun dato ricevuto");
    }
  });

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", index_html);
  });

  server.onNotFound([]() {
    server.send_P(200, "text/html", index_html);
  });

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
