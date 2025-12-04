#include <WiFi.h>
#include <WebServer.h>

#define LDR_PIN 34
#define SOIL_PIN 35
#define RELAY_PIN 26

// --- Access Point ---
const char* ap_ssid  = "SmartPlant_AP";
const char* ap_pass  = "12345678";

// --- Variables del sistema ---
int relayState = HIGH;
unsigned long lastWatering = 0;

// Valores iniciales (modo TEST)
unsigned long wateringDelay = 10UL * 1000UL;
int wateringDuration = 900;
int umbralHumedad = 70;
int umbralLuz = 50;

// Bomba NO bloqueante
bool bombaActiva = false;
unsigned long bombaInicio = 0;

WebServer server(80);

// Prototipos
void activarBomba();
void actualizarBomba();
void handleRoot();
void handleData();
void handleTogglePump();
void handleSetPlant();


// ===========================
//          SETUP
// ===========================
void setup() {
  Serial.begin(115200);

  pinMode(LDR_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  // Access Point
  WiFi.softAP(ap_ssid, ap_pass);
  delay(200);

  Serial.println("AP iniciado en: ");
  Serial.println(WiFi.softAPIP());

  // Rutas HTTP
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/toggle", handleTogglePump);
  server.on("/setplant", handleSetPlant);

  server.begin();
  Serial.println("Servidor listo.");
}


// ===========================
//           LOOP
// ===========================

void loop() {
  server.handleClient();
  actualizarBomba();

  int soilValue = analogRead(SOIL_PIN);
  int humedad = map(soilValue, 0, 4095, 100, 0);

  int ldrValue = analogRead(LDR_PIN);
  int luz = map(ldrValue, 0, 4095, 100, 0);

  unsigned long now = millis();

  // Riego automático
  if (!bombaActiva &&
      humedad < umbralHumedad &&
      luz > umbralLuz &&
      (now - lastWatering > wateringDelay))
  {
    Serial.println("💧 Riego automático...");
    activarBomba();
  }
}


// ============================================================
//         BOMBA NO BLOQUEANTE
// ============================================================
void activarBomba() {
  bombaActiva = true;
  bombaInicio = millis();

  relayState = LOW;
  digitalWrite(RELAY_PIN, LOW);

  Serial.println("💦 Bomba encendida...");
}

void actualizarBomba() {
  if (bombaActiva) {
    if (millis() - bombaInicio >= wateringDuration) {
      bombaActiva = false;
      relayState = HIGH;
      digitalWrite(RELAY_PIN, HIGH);

      Serial.println("🚫 Bomba apagada.");

      lastWatering = millis();
    }
  }
}

void handleTogglePump() {
  if (!bombaActiva)
    activarBomba();

  server.send(200, "text/plain", "OK");
}



// ============================================================
//       CARGAR CONFIG SEGÚN PLANTA
// ============================================================
void handleSetPlant() {
  if (!server.hasArg("type")) {
    server.send(400, "text/plain", "missing type");
    return;
  }

  String t = server.arg("type");

  if (t == "test") {
    wateringDelay = 10UL * 1000UL;
    wateringDuration = 5000;
    umbralHumedad = 70;
  }
  else if (t == "suculenta") {
    wateringDelay = 48UL * 60UL * 60UL * 1000UL;
    wateringDuration = 2000;
    umbralHumedad = 20;
  }
  else if (t == "potus") {
    wateringDelay = 12UL * 60UL * 60UL * 1000UL;
    wateringDuration = 5000;
    umbralHumedad = 50;
  }
  else if (t == "tomate") {
    wateringDelay = 8UL * 60UL * 60UL * 1000UL;
    wateringDuration = 6000;
    umbralHumedad = 60;
  }
  else if (t == "helecho") {
    wateringDelay = 6UL * 60UL * 60UL * 1000UL;
    wateringDuration = 8000;
    umbralHumedad = 70;
  }

  Serial.print("🌱 Planta seleccionada: ");
  Serial.println(t);

  server.send(200, "text/plain", "OK");
}



// ============================================================
// HTML + JS (CON GRÁFICOS)
// ============================================================
void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="es">
  <head>
    <meta charset="UTF-8">
    <title>SmartPlant Dashboard</title>

    <style>
      body { font-family: Arial; text-align: center; background: #f4f4f4; padding: 20px; }
      .card { background: white; padding: 20px; border-radius: 15px; box-shadow: 0 0 10px #ccc; max-width: 400px; margin: auto; }
      .btn { padding: 10px 20px; background: #2c7a7b; color: white; border: none; border-radius: 10px; margin-top: 10px; }
      .charts-container { display: flex; justify-content: center; gap: 10px; flex-wrap: wrap; margin-top: 20px; }
      canvas { background: white; border-radius: 10px; box-shadow: 0 0 5px #ccc; }
    </style>
  </head>

  <body>
    <h1>🌿 SmartPlant Dashboard</h1>

    <div class="card">
      <p><strong>Luz:</strong> <span id="ldr">0</span>%</p>
      <p><strong>Humedad:</strong> <span id="soil">0</span>%</p>
      <p><strong>Bomba:</strong> <span id="pump">OFF</span></p>

      <button class="btn" onclick="togglePump()">Riego Manual</button>

      <h3>🌱 Tipo de Planta</h3>
      <select id="plant" onchange="setPlant()">
        <option value="test">TEST (actual)</option>
        <option value="suculenta">Suculenta</option>
        <option value="potus">Potus</option>
        <option value="tomate">Tomate</option>
        <option value="helecho">Helecho</option>
      </select>
    </div>

    <div class="charts-container">
      <div>
        <p><strong>Luz</strong></p>
        <canvas id="chartLuz" width="320" height="180"></canvas>
      </div>

      <div>
        <p><strong>Humedad</strong></p>
        <canvas id="chartHum" width="320" height="180"></canvas>
      </div>
    </div>

    <script>
      // Buffers
      const MAX_POINTS = 30;
      const dataLuz = [];
      const dataHum = [];

      function pushWithLimit(arr, val) {
        arr.push(val);
        if (arr.length > MAX_POINTS) arr.shift();
      }

      // Dibujo de gráficos
      function drawChart(canvasId, data, color, label) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const w = canvas.width;
        const h = canvas.height;

        ctx.clearRect(0,0,w,h);

        const ml = 30, mr = 10, mt = 10, mb = 25;

        // ejes
        ctx.strokeStyle = "#ccc";
        ctx.beginPath();
        ctx.moveTo(ml, mt);
        ctx.lineTo(ml, h-mb);
        ctx.lineTo(w-mr, h-mb);
        ctx.stroke();

        // labels Y
        ctx.font = "10px Arial";
        ctx.fillStyle = "#333";
        [0,50,100].forEach(v => {
          const y = (h-mb) - (v/100)*(h-mt-mb);
          ctx.fillText(v, 5, y+3);
          ctx.strokeStyle="#eee";
          ctx.beginPath();
          ctx.moveTo(ml, y);
          ctx.lineTo(w-mr, y);
          ctx.stroke();
        });

        if (data.length < 2) return;

        const step = (w-ml-mr)/(MAX_POINTS-1);

        ctx.strokeStyle = color;
        ctx.lineWidth = 2;
        ctx.beginPath();

        data.forEach((v,i) => {
          const x = ml + i*step;
          const y = (h-mb) - (v/100)*(h-mt-mb);
          if(i==0) ctx.moveTo(x,y);
          else ctx.lineTo(x,y);
        });

        ctx.stroke();
      }

      // Actualización sensores
      async function updateData() {
        const res = await fetch('/data');
        const data = await res.json();

        document.getElementById('ldr').textContent = data.ldr;
        document.getElementById('soil').textContent = data.soil;
        document.getElementById('pump').textContent = data.pump ? "ON" : "OFF";

        pushWithLimit(dataLuz, data.ldr);
        pushWithLimit(dataHum, data.soil);

        drawChart("chartLuz", dataLuz, "#ff8800", "Luz");
        drawChart("chartHum", dataHum, "#0077ff", "Humedad");
      }

      async function togglePump() {
        await fetch('/toggle');
        updateData();
      }

      async function setPlant() {
        const plant = document.getElementById("plant").value;
        await fetch('/setplant?type=' + plant);
      }

      setInterval(updateData, 500);
      updateData();
    </script>

  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}



// ============================================================
// API JSON
// ============================================================
void handleData() {
  int luz = map(analogRead(LDR_PIN), 0, 4095, 100, 0);
  int humedad = map(analogRead(SOIL_PIN), 0, 4095, 100, 0);

  String json = "{\"ldr\":"+String(luz)+
                ",\"soil\":"+String(humedad)+
                ",\"pump\":"+(relayState==LOW ? String("true") : String("false"))+
                "}";

  server.send(200, "application/json", json);
}
