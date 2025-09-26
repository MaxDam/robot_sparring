// server.js — mini-server HTTPS Node.js per getUserMedia
const fs = require('fs');
const https = require('https');
const express = require('express');
const path = require('path');
const os = require('os');

const app = express();
const PORT = 8000;

// Serve file statici dalla cartella corrente
app.use(express.static(path.join(__dirname)));

// Leggi certificati
const options = {
  key: fs.readFileSync(path.join(__dirname, 'key.pem')),
  cert: fs.readFileSync(path.join(__dirname, 'cert.pem'))
};

// Trova IP LAN nella subnet 192.168.x.x
function getLAN192IP() {
  const nets = os.networkInterfaces();
  for (const name of Object.keys(nets)) {
    for (const net of nets[name]) {
      if (net.family === 'IPv4' && !net.internal) {
        if (net.address.startsWith('192.168.')) {
          return net.address;
        }
      }
    }
  }
  return 'localhost';
}

// Avvia server HTTPS
https.createServer(options, app).listen(PORT, () => {
  const localIP = getLAN192IP();
  console.log(`Apri sul tuo dispositivo Android: https://${localIP}:${PORT}`);
});

//C:\OpenSSL\SSL\openssl req -nodes -new -x509 -keyout key.pem -out cert.pem -days 365
//npm init -y
//npm install express
//node server.js