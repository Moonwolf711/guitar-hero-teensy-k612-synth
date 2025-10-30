/**
 * ESP8266 WiFi Control Module
 * Provides web interface and OSC control for Guitar Hero Synthesizer
 *
 * Features:
 * - Access Point mode for standalone operation
 * - Web interface for parameter control
 * - OSC message handling
 * - Serial communication with Teensy
 * - JSON-based command protocol
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>

// Configuration
const char* AP_SSID = "GuitarHeroSynth";
const char* AP_PASS = "music123";
const char* HOSTNAME = "guitarhero";

// Serial communication with Teensy
#define TEENSY_SERIAL Serial
#define TEENSY_BAUD 115200

// Web server
ESP8266WebServer server(80);

// OSC
WiFiUDP oscUdp;
const unsigned int OSC_PORT = 8000;
const unsigned int OSC_REPLY_PORT = 8001;

// System state
struct SystemState {
    bool controllerConnected;
    uint8_t currentScale;
    int8_t octaveShift;
    float cpuUsage;
    uint8_t memoryUsage;
    uint8_t activeVoices;
    char lastMessage[128];
} state;

// JSON document for parsing
StaticJsonDocument<512> jsonDoc;

// Function prototypes
void setupWiFi();
void setupWebServer();
void setupOSC();
void handleRoot();
void handleStatus();
void handleControl();
void handleNotFound();
void processSerialCommand();
void sendTeensyCommand(const char* cmd);
void handleOSCMessage(OSCMessage &msg);
void updateState(const char* json);

// HTML content (stored in PROGMEM to save RAM)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Guitar Hero Synthesizer</title>
    <style>
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            margin: 0;
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        h1 {
            text-align: center;
            font-size: 2.5em;
            margin-bottom: 30px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .status {
            background: rgba(255,255,255,0.1);
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
            backdrop-filter: blur(10px);
        }
        .status h2 {
            margin-top: 0;
            color: #ffd700;
        }
        .status-item {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid rgba(255,255,255,0.2);
        }
        .status-item:last-child {
            border-bottom: none;
        }
        .status-value {
            font-weight: bold;
            color: #00ff88;
        }
        .controls {
            background: rgba(255,255,255,0.1);
            border-radius: 10px;
            padding: 20px;
            backdrop-filter: blur(10px);
        }
        .control-group {
            margin-bottom: 20px;
        }
        .control-group label {
            display: block;
            margin-bottom: 5px;
            color: #ffd700;
            font-weight: bold;
        }
        select, input[type="range"], button {
            width: 100%;
            padding: 10px;
            border: none;
            border-radius: 5px;
            background: rgba(255,255,255,0.2);
            color: white;
            font-size: 16px;
        }
        select option {
            background: #764ba2;
        }
        input[type="range"] {
            -webkit-appearance: none;
            height: 10px;
            background: rgba(255,255,255,0.3);
            outline: none;
        }
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 25px;
            height: 25px;
            background: #ffd700;
            cursor: pointer;
            border-radius: 50%;
        }
        button {
            background: #ffd700;
            color: #764ba2;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s;
            margin-top: 10px;
        }
        button:hover {
            background: #ffed4e;
            transform: translateY(-2px);
            box-shadow: 0 5px 10px rgba(0,0,0,0.3);
        }
        .range-value {
            text-align: center;
            margin-top: 5px;
            font-size: 0.9em;
        }
        .connection-status {
            width: 15px;
            height: 15px;
            border-radius: 50%;
            display: inline-block;
            margin-left: 10px;
        }
        .connected {
            background-color: #00ff88;
            animation: pulse 2s infinite;
        }
        .disconnected {
            background-color: #ff4444;
        }
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎸 Guitar Hero Synthesizer</h1>

        <div class="status">
            <h2>System Status</h2>
            <div class="status-item">
                <span>Controller</span>
                <span class="status-value">
                    <span id="controller-status">Disconnected</span>
                    <span id="connection-indicator" class="connection-status disconnected"></span>
                </span>
            </div>
            <div class="status-item">
                <span>Current Scale</span>
                <span class="status-value" id="current-scale">Pentatonic Minor</span>
            </div>
            <div class="status-item">
                <span>Octave Shift</span>
                <span class="status-value" id="octave-shift">0</span>
            </div>
            <div class="status-item">
                <span>CPU Usage</span>
                <span class="status-value" id="cpu-usage">0%</span>
            </div>
            <div class="status-item">
                <span>Active Voices</span>
                <span class="status-value" id="active-voices">0/6</span>
            </div>
        </div>

        <div class="controls">
            <h2>Controls</h2>

            <div class="control-group">
                <label for="scale-select">Scale</label>
                <select id="scale-select" onchange="changeScale(this.value)">
                    <option value="0">Pentatonic Minor</option>
                    <option value="1">Natural Minor</option>
                    <option value="2">Dorian</option>
                    <option value="3">Hungarian Minor</option>
                    <option value="4">Harmonic Minor</option>
                    <option value="5">Phrygian</option>
                </select>
            </div>

            <div class="control-group">
                <label for="reverb-mix">Reverb Mix</label>
                <input type="range" id="reverb-mix" min="0" max="100" value="30"
                       oninput="updateValue('reverb', this.value)">
                <div class="range-value" id="reverb-value">30%</div>
            </div>

            <div class="control-group">
                <label for="delay-mix">Delay Mix</label>
                <input type="range" id="delay-mix" min="0" max="100" value="20"
                       oninput="updateValue('delay', this.value)">
                <div class="range-value" id="delay-value">20%</div>
            </div>

            <div class="control-group">
                <label for="filter-freq">Filter Frequency</label>
                <input type="range" id="filter-freq" min="100" max="4000" value="2000"
                       oninput="updateValue('filter', this.value)">
                <div class="range-value" id="filter-value">2000 Hz</div>
            </div>

            <button onclick="resetDefaults()">Reset to Defaults</button>
            <button onclick="savePreset()">Save Preset</button>
        </div>
    </div>

    <script>
        // Update status every second
        setInterval(updateStatus, 1000);

        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('controller-status').textContent =
                        data.connected ? 'Connected' : 'Disconnected';

                    const indicator = document.getElementById('connection-indicator');
                    indicator.className = 'connection-status ' +
                        (data.connected ? 'connected' : 'disconnected');

                    document.getElementById('current-scale').textContent =
                        getScaleName(data.scale);
                    document.getElementById('octave-shift').textContent =
                        (data.octave > 0 ? '+' : '') + data.octave;
                    document.getElementById('cpu-usage').textContent =
                        data.cpu.toFixed(1) + '%';
                    document.getElementById('active-voices').textContent =
                        data.voices + '/6';
                })
                .catch(err => console.error('Status update failed:', err));
        }

        function getScaleName(index) {
            const scales = [
                'Pentatonic Minor', 'Natural Minor', 'Dorian',
                'Hungarian Minor', 'Harmonic Minor', 'Phrygian'
            ];
            return scales[index] || 'Unknown';
        }

        function changeScale(value) {
            fetch('/control', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({command: 'setScale', value: parseInt(value)})
            });
        }

        function updateValue(param, value) {
            document.getElementById(param + '-value').textContent =
                param === 'filter' ? value + ' Hz' : value + '%';

            fetch('/control', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({command: 'set' + param, value: parseFloat(value)})
            });
        }

        function resetDefaults() {
            document.getElementById('reverb-mix').value = 30;
            document.getElementById('delay-mix').value = 20;
            document.getElementById('filter-freq').value = 2000;
            updateValue('reverb', 30);
            updateValue('delay', 20);
            updateValue('filter', 2000);
        }

        function savePreset() {
            fetch('/control', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({command: 'savePreset'})
            }).then(() => alert('Preset saved!'));
        }

        // Initial status update
        updateStatus();
    </script>
</body>
</html>
)rawliteral";

void setup() {
    // Initialize serial communication with Teensy
    TEENSY_SERIAL.begin(TEENSY_BAUD);

    // Initialize file system
    if (!LittleFS.begin()) {
        Serial.println(F("Failed to mount file system"));
    }

    // Initialize state
    memset(&state, 0, sizeof(state));
    strcpy(state.lastMessage, "System initialized");

    // Setup WiFi
    setupWiFi();

    // Setup web server
    setupWebServer();

    // Setup OSC
    setupOSC();

    // Start mDNS
    if (MDNS.begin(HOSTNAME)) {
        Serial.println(F("mDNS responder started"));
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("osc", "udp", OSC_PORT);
    }

    Serial.println(F("ESP8266 WiFi module ready"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.softAPIP());
}

void loop() {
    // Handle web server
    server.handleClient();

    // Handle mDNS
    MDNS.update();

    // Process serial commands from Teensy
    if (TEENSY_SERIAL.available()) {
        processSerialCommand();
    }

    // Check for OSC messages
    OSCMessage msg;
    int size = oscUdp.parsePacket();
    if (size > 0) {
        while (size--) {
            msg.fill(oscUdp.read());
        }
        if (!msg.hasError()) {
            handleOSCMessage(msg);
        }
    }

    // Small delay to prevent watchdog
    yield();
}

void setupWiFi() {
    Serial.println(F("Setting up WiFi Access Point..."));

    // Configure AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);

    // Set static IP
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    Serial.print(F("AP SSID: "));
    Serial.println(AP_SSID);
    Serial.print(F("AP Password: "));
    Serial.println(AP_PASS);
}

void setupWebServer() {
    // Route handlers
    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/control", HTTP_POST, handleControl);
    server.onNotFound(handleNotFound);

    // Start server
    server.begin();
    Serial.println(F("HTTP server started on port 80"));
}

void setupOSC() {
    oscUdp.begin(OSC_PORT);
    Serial.print(F("OSC server started on port "));
    Serial.println(OSC_PORT);
}

void handleRoot() {
    server.send_P(200, "text/html", index_html);
}

void handleStatus() {
    // Create JSON response
    StaticJsonDocument<256> doc;
    doc["connected"] = state.controllerConnected;
    doc["scale"] = state.currentScale;
    doc["octave"] = state.octaveShift;
    doc["cpu"] = state.cpuUsage;
    doc["memory"] = state.memoryUsage;
    doc["voices"] = state.activeVoices;
    doc["message"] = state.lastMessage;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleControl() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");

        DeserializationError error = deserializeJson(jsonDoc, body);
        if (!error) {
            const char* command = jsonDoc["command"];

            // Build command for Teensy
            StaticJsonDocument<128> cmdDoc;
            cmdDoc["cmd"] = command;

            if (jsonDoc.containsKey("value")) {
                cmdDoc["value"] = jsonDoc["value"];
            }

            String cmd;
            serializeJson(cmdDoc, cmd);
            sendTeensyCommand(cmd.c_str());

            server.send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No data\"}");
    }
}

void handleNotFound() {
    server.send(404, "text/plain", "Not Found");
}

void processSerialCommand() {
    static char buffer[256];
    static uint8_t index = 0;

    while (TEENSY_SERIAL.available()) {
        char c = TEENSY_SERIAL.read();

        if (c == '\n' || c == '\r') {
            if (index > 0) {
                buffer[index] = '\0';
                updateState(buffer);
                index = 0;
            }
        } else if (index < 255) {
            buffer[index++] = c;
        }
    }
}

void sendTeensyCommand(const char* cmd) {
    TEENSY_SERIAL.println(cmd);
}

void updateState(const char* json) {
    DeserializationError error = deserializeJson(jsonDoc, json);
    if (!error) {
        if (jsonDoc.containsKey("connected")) {
            state.controllerConnected = jsonDoc["connected"];
        }
        if (jsonDoc.containsKey("scale")) {
            state.currentScale = jsonDoc["scale"];
        }
        if (jsonDoc.containsKey("octave")) {
            state.octaveShift = jsonDoc["octave"];
        }
        if (jsonDoc.containsKey("cpu")) {
            state.cpuUsage = jsonDoc["cpu"];
        }
        if (jsonDoc.containsKey("mem")) {
            state.memoryUsage = jsonDoc["mem"];
        }
        if (jsonDoc.containsKey("voices")) {
            state.activeVoices = jsonDoc["voices"];
        }
    }
}

void handleOSCMessage(OSCMessage &msg) {
    char address[64];
    msg.getAddress(address);

    // Handle different OSC addresses
    if (msg.match("/scale")) {
        if (msg.isInt(0)) {
            int scale = msg.getInt(0);
            StaticJsonDocument<64> doc;
            doc["cmd"] = "setScale";
            doc["value"] = scale;
            String cmd;
            serializeJson(doc, cmd);
            sendTeensyCommand(cmd.c_str());
        }
    } else if (msg.match("/reverb")) {
        if (msg.isFloat(0)) {
            float value = msg.getFloat(0);
            StaticJsonDocument<64> doc;
            doc["cmd"] = "setReverb";
            doc["value"] = value;
            String cmd;
            serializeJson(doc, cmd);
            sendTeensyCommand(cmd.c_str());
        }
    } else if (msg.match("/delay")) {
        if (msg.isFloat(0)) {
            float value = msg.getFloat(0);
            StaticJsonDocument<64> doc;
            doc["cmd"] = "setDelay";
            doc["value"] = value;
            String cmd;
            serializeJson(doc, cmd);
            sendTeensyCommand(cmd.c_str());
        }
    }
}