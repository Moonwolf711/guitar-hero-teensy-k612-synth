/*
 * ESP-12E WiFi Control + OSC for Guitar Hero Synth
 * Real-time web interface + OSC broadcasting
 *
 * Features:
 * - Web interface with live visualization
 * - OSC output (UDP port 8000)
 * - Real-time note display
 * - CPU/Memory monitoring
 * - Configuration persistence
 * - Mobile-friendly UI
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

// ===== CONFIGURATION =====
const char* AP_SSID = "GuitarHero-Synth";
const char* AP_PASSWORD = "rockstar";
const char* MDNS_NAME = "gh-synth";

// OSC Configuration
#define OSC_ENABLED true
#define OSC_PORT 8000
WiFiUDP udp;
IPAddress oscTargetIP(255, 255, 255, 255);  // Broadcast
uint16_t oscTargetPort = OSC_PORT;

// Web Server
ESP8266WebServer server(80);

// EEPROM Storage
#define CONFIG_ADDR 0
struct Config {
  uint8_t scale;
  uint8_t root;
  int8_t octave;
  bool arpEnabled;
  uint16_t arpSpeed;
  char staSSID[32];
  char staPassword[64];
  bool apMode;
  IPAddress oscTarget;
  uint16_t checksum;
};

Config config;

// State from Teensy
struct SynthState {
  uint8_t currentScale;
  char scaleName[32];
  uint8_t currentRoot;
  char rootName[8];
  int8_t octave;
  bool arpActive;
  uint8_t lastNote;
  bool noteOn;
  float cpuUsage;
  uint16_t memUsage;
  uint32_t totalNotes;
  uint32_t latency;
  unsigned long lastUpdate;
} synthState;

// ===== OSC FUNCTIONS =====

void sendOSC(const char* address, const char* types, ...) {
  if (!OSC_ENABLED) return;

  // Simple OSC message format
  // Format: /address\0,types\0data...
  uint8_t buffer[256];
  int pos = 0;

  // Address
  int addrLen = strlen(address) + 1;
  memcpy(buffer + pos, address, addrLen);
  pos += addrLen;

  // Align to 4 bytes
  while (pos % 4 != 0) buffer[pos++] = 0;

  // Type tag
  buffer[pos++] = ',';
  int typesLen = strlen(types);
  memcpy(buffer + pos, types, typesLen);
  pos += typesLen;
  buffer[pos++] = 0;

  // Align to 4 bytes
  while (pos % 4 != 0) buffer[pos++] = 0;

  // Arguments
  va_list args;
  va_start(args, types);

  for (int i = 0; i < typesLen; i++) {
    switch (types[i]) {
      case 'i':  // int32
        {
          int32_t val = va_arg(args, int32_t);
          buffer[pos++] = (val >> 24) & 0xFF;
          buffer[pos++] = (val >> 16) & 0xFF;
          buffer[pos++] = (val >> 8) & 0xFF;
          buffer[pos++] = val & 0xFF;
        }
        break;

      case 'f':  // float32
        {
          union {
            float f;
            uint32_t i;
          } u;
          u.f = (float)va_arg(args, double);
          buffer[pos++] = (u.i >> 24) & 0xFF;
          buffer[pos++] = (u.i >> 16) & 0xFF;
          buffer[pos++] = (u.i >> 8) & 0xFF;
          buffer[pos++] = u.i & 0xFF;
        }
        break;

      case 's':  // string
        {
          const char* str = va_arg(args, const char*);
          int strLen = strlen(str) + 1;
          memcpy(buffer + pos, str, strLen);
          pos += strLen;
          while (pos % 4 != 0) buffer[pos++] = 0;
        }
        break;
    }
  }

  va_end(args);

  // Send UDP packet
  udp.beginPacket(oscTargetIP, oscTargetPort);
  udp.write(buffer, pos);
  udp.endPacket();
}

// ===== TEENSY COMMUNICATION =====

void processTeensyData() {
  if (!Serial.available()) return;

  String data = Serial.readStringUntil('\n');
  data.trim();

  if (data.length() == 0) return;

  // Parse JSON from Teensy
  if (data.startsWith("{\"type\":\"config\"")) {
    // Config update
    int scalePos = data.indexOf("\"scale\":");
    if (scalePos >= 0) {
      synthState.currentScale = data.substring(scalePos + 8).toInt();
    }

    int scaleNamePos = data.indexOf("\"scaleName\":\"");
    if (scaleNamePos >= 0) {
      int endPos = data.indexOf("\"", scaleNamePos + 13);
      String name = data.substring(scaleNamePos + 13, endPos);
      strncpy(synthState.scaleName, name.c_str(), 31);
    }

    int rootPos = data.indexOf("\"root\":");
    if (rootPos >= 0) {
      synthState.currentRoot = data.substring(rootPos + 7).toInt();
    }

    int rootNamePos = data.indexOf("\"rootName\":\"");
    if (rootNamePos >= 0) {
      int endPos = data.indexOf("\"", rootNamePos + 12);
      String name = data.substring(rootNamePos + 12, endPos);
      strncpy(synthState.rootName, name.c_str(), 7);
    }

    int octavePos = data.indexOf("\"octave\":");
    if (octavePos >= 0) {
      synthState.octave = data.substring(octavePos + 9).toInt();
    }

    synthState.arpActive = data.indexOf("\"arp\":true") >= 0;

    // Send OSC
    sendOSC("/synth/scale", "is", synthState.currentScale, synthState.scaleName);
    sendOSC("/synth/root", "is", synthState.currentRoot, synthState.rootName);
    sendOSC("/synth/octave", "i", synthState.octave);
    sendOSC("/synth/arp", "i", synthState.arpActive ? 1 : 0);

    synthState.lastUpdate = millis();
  }
  else if (data.startsWith("{\"type\":\"note\"")) {
    // Note on/off
    int notePos = data.indexOf("\"note\":");
    if (notePos >= 0) {
      synthState.lastNote = data.substring(notePos + 7).toInt();
    }

    synthState.noteOn = data.indexOf("\"on\":true") >= 0;

    // Send OSC
    if (synthState.noteOn) {
      sendOSC("/synth/noteon", "i", synthState.lastNote);
    } else {
      sendOSC("/synth/noteoff", "i", synthState.lastNote);
    }

    synthState.lastUpdate = millis();
  }
  else if (data.startsWith("{\"type\":\"status\"")) {
    // Status update
    int cpuPos = data.indexOf("\"cpu\":");
    if (cpuPos >= 0) {
      synthState.cpuUsage = data.substring(cpuPos + 6).toFloat();
    }

    int memPos = data.indexOf("\"mem\":");
    if (memPos >= 0) {
      synthState.memUsage = data.substring(memPos + 6).toInt();
    }

    int notesPos = data.indexOf("\"notes\":");
    if (notesPos >= 0) {
      synthState.totalNotes = data.substring(notesPos + 8).toInt();
    }

    int latencyPos = data.indexOf("\"latency\":");
    if (latencyPos >= 0) {
      synthState.latency = data.substring(latencyPos + 10).toInt();
    }

    // Send OSC
    sendOSC("/synth/cpu", "f", synthState.cpuUsage);
    sendOSC("/synth/memory", "i", synthState.memUsage);
    sendOSC("/synth/latency", "i", synthState.latency);

    synthState.lastUpdate = millis();
  }
}

// ===== WEB SERVER =====

String getWebPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Guitar Hero Synth Control</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
      color: white;
      padding: 20px;
      min-height: 100vh;
    }
    .container {
      max-width: 1200px;
      margin: 0 auto;
    }
    .header {
      text-align: center;
      margin-bottom: 30px;
    }
    .header h1 {
      font-size: 36px;
      margin-bottom: 10px;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
    }
    .card {
      background: rgba(255, 255, 255, 0.1);
      backdrop-filter: blur(10px);
      border-radius: 15px;
      padding: 20px;
      box-shadow: 0 8px 32px rgba(0,0,0,0.3);
    }
    .card h2 {
      font-size: 20px;
      margin-bottom: 15px;
      border-bottom: 2px solid rgba(255,255,255,0.3);
      padding-bottom: 10px;
    }
    .note-display {
      text-align: center;
      font-size: 72px;
      font-weight: bold;
      padding: 30px;
      background: rgba(0,0,0,0.3);
      border-radius: 10px;
      min-height: 150px;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    .note-on {
      animation: pulse 0.5s infinite;
      color: #4ade80;
    }
    @keyframes pulse {
      0%, 100% { transform: scale(1); }
      50% { transform: scale(1.1); }
    }
    .info-row {
      display: flex;
      justify-content: space-between;
      padding: 10px 0;
      border-bottom: 1px solid rgba(255,255,255,0.1);
    }
    .info-label {
      opacity: 0.7;
    }
    .info-value {
      font-weight: bold;
    }
    .btn {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border: none;
      color: white;
      padding: 12px 24px;
      border-radius: 8px;
      cursor: pointer;
      font-size: 16px;
      margin: 5px;
      transition: transform 0.2s;
    }
    .btn:hover {
      transform: translateY(-2px);
    }
    .btn:active {
      transform: translateY(0);
    }
    .controls {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
    }
    .meter {
      background: rgba(0,0,0,0.3);
      border-radius: 10px;
      height: 30px;
      position: relative;
      overflow: hidden;
    }
    .meter-fill {
      height: 100%;
      background: linear-gradient(90deg, #4ade80 0%, #22c55e 100%);
      transition: width 0.3s;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: bold;
    }
    .status-indicator {
      width: 12px;
      height: 12px;
      border-radius: 50%;
      display: inline-block;
      margin-right: 8px;
    }
    .status-online {
      background: #4ade80;
      box-shadow: 0 0 10px #4ade80;
    }
    .status-offline {
      background: #ef4444;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>🎸 Guitar Hero Synth Control</h1>
      <p>
        <span class="status-indicator" id="status"></span>
        <span id="statusText">Connecting...</span>
      </p>
    </div>

    <div class="grid">
      <!-- Note Display -->
      <div class="card">
        <h2>🎵 Current Note</h2>
        <div class="note-display" id="noteDisplay">---</div>
      </div>

      <!-- Performance -->
      <div class="card">
        <h2>⚡ Performance</h2>
        <div class="info-row">
          <span class="info-label">CPU Usage:</span>
          <span class="info-value" id="cpu">0%</span>
        </div>
        <div class="meter">
          <div class="meter-fill" id="cpuMeter" style="width: 0%">
            <span id="cpuText">0%</span>
          </div>
        </div>
        <div class="info-row">
          <span class="info-label">Memory:</span>
          <span class="info-value" id="mem">0 blocks</span>
        </div>
        <div class="info-row">
          <span class="info-label">Latency:</span>
          <span class="info-value" id="latency">0 μs</span>
        </div>
        <div class="info-row">
          <span class="info-label">Total Notes:</span>
          <span class="info-value" id="notes">0</span>
        </div>
      </div>

      <!-- Configuration -->
      <div class="card">
        <h2>🎹 Configuration</h2>
        <div class="info-row">
          <span class="info-label">Scale:</span>
          <span class="info-value" id="scale">---</span>
        </div>
        <div class="info-row">
          <span class="info-label">Root Note:</span>
          <span class="info-value" id="root">---</span>
        </div>
        <div class="info-row">
          <span class="info-label">Octave:</span>
          <span class="info-value" id="octave">0</span>
        </div>
        <div class="info-row">
          <span class="info-label">Arpeggiator:</span>
          <span class="info-value" id="arp">OFF</span>
        </div>
      </div>

      <!-- Quick Controls -->
      <div class="card">
        <h2>🎛️ Quick Controls</h2>
        <div class="controls">
          <button class="btn" onclick="sendCmd('scale', 0)">Major Pent</button>
          <button class="btn" onclick="sendCmd('scale', 1)">Minor Pent</button>
          <button class="btn" onclick="sendCmd('scale', 2)">Blues</button>
          <button class="btn" onclick="sendCmd('scale', 3)">Japanese</button>
          <button class="btn" onclick="sendCmd('octave', 1)">Octave +</button>
          <button class="btn" onclick="sendCmd('octave', -1)">Octave -</button>
          <button class="btn" onclick="toggleArp()">Toggle Arp</button>
          <button class="btn" onclick="location.reload()">Refresh</button>
        </div>
      </div>
    </div>
  </div>

  <script>
    let lastUpdate = 0;

    function updateStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('status').className = 'status-indicator status-online';
          document.getElementById('statusText').textContent = 'Connected';

          if (data.noteOn) {
            document.getElementById('noteDisplay').textContent = data.noteName || data.lastNote;
            document.getElementById('noteDisplay').className = 'note-display note-on';
          } else {
            document.getElementById('noteDisplay').textContent = '---';
            document.getElementById('noteDisplay').className = 'note-display';
          }

          document.getElementById('cpu').textContent = data.cpu.toFixed(1) + '%';
          document.getElementById('cpuMeter').style.width = data.cpu + '%';
          document.getElementById('cpuText').textContent = data.cpu.toFixed(1) + '%';

          document.getElementById('mem').textContent = data.mem + ' blocks';
          document.getElementById('latency').textContent = data.latency + ' μs';
          document.getElementById('notes').textContent = data.notes;

          document.getElementById('scale').textContent = data.scale;
          document.getElementById('root').textContent = data.root;
          document.getElementById('octave').textContent = data.octave;
          document.getElementById('arp').textContent = data.arp ? 'ON' : 'OFF';

          lastUpdate = Date.now();
        })
        .catch(err => {
          if (Date.now() - lastUpdate > 3000) {
            document.getElementById('status').className = 'status-indicator status-offline';
            document.getElementById('statusText').textContent = 'Disconnected';
          }
        });
    }

    function sendCmd(cmd, value) {
      fetch('/api/cmd', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({cmd: cmd, value: value})
      });
    }

    function toggleArp() {
      fetch('/api/cmd', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({cmd: 'arp'})
      });
    }

    // Update every 100ms
    setInterval(updateStatus, 100);
    updateStatus();
  </script>
</body>
</html>
)rawliteral";
}

void handleRoot() {
  server.send(200, "text/html", getWebPage());
}

void handleStatus() {
  String json = "{";
  json += "\"noteOn\":" + String(synthState.noteOn ? "true" : "false") + ",";
  json += "\"lastNote\":" + String(synthState.lastNote) + ",";
  json += "\"noteName\":\"" + String(synthState.rootName) + "\",";
  json += "\"cpu\":" + String(synthState.cpuUsage, 1) + ",";
  json += "\"mem\":" + String(synthState.memUsage) + ",";
  json += "\"latency\":" + String(synthState.latency) + ",";
  json += "\"notes\":" + String(synthState.totalNotes) + ",";
  json += "\"scale\":\"" + String(synthState.scaleName) + "\",";
  json += "\"root\":\"" + String(synthState.rootName) + "\",";
  json += "\"octave\":" + String(synthState.octave) + ",";
  json += "\"arp\":" + String(synthState.arpActive ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

void handleCommand() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "No data");
    return;
  }

  String body = server.arg("plain");

  // Forward command to Teensy
  Serial.println(body);

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

// ===== MAIN =====

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=== ESP-12E OSC Control ===");

  // Initialize WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(IP);

  // Initialize mDNS
  if (MDNS.begin(MDNS_NAME)) {
    Serial.print("mDNS: http://");
    Serial.print(MDNS_NAME);
    Serial.println(".local");
  }

  // Initialize UDP for OSC
  udp.begin(OSC_PORT);
  Serial.print("OSC: Port ");
  Serial.println(OSC_PORT);

  // Setup web server
  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/cmd", HTTP_POST, handleCommand);
  server.begin();

  Serial.println("Web server started");
  Serial.println("Ready!");
}

void loop() {
  server.handleClient();
  MDNS.update();

  // Process data from Teensy
  processTeensyData();

  delay(1);
}
