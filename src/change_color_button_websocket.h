#include <WiFi.h>
#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* SSID = "DarkSideOfTheMoon";
const char* PASSWORD = "maytheforcebewithyou";


AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket endpoint
bool state = false;
unsigned long previousMillis = 0;   // Tracks the last time the state was toggled
const long interval = 2000;         // Toggle interval (2 seconds)

// Serve the webpage with WebSocket support
void handleRoot(AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head><style>";
    html += ".button { padding: 15px; font-size: 16px; }";
    html += ".on { background-color: green; color: white; }";
    html += ".off { background-color: red; color: white; }";
    html += "</style></head><body>";
    html += "<h1>ESP32 State Button</h1>";
    html += "<button id='stateButton' class='button ";
    html += (state ? "on" : "off");
    html += "'>State is ";
    html += (state ? "ON" : "OFF");
    html += "</button>";
    html += "<script>";
    html += "var socket = new WebSocket('ws://' + location.host + '/ws');";
    html += "socket.onmessage = function(event) {";
    html += "var data = JSON.parse(event.data);";
    html += "document.getElementById('stateButton').className = 'button ' + (data.state ? 'on' : 'off');";
    html += "document.getElementById('stateButton').innerText = 'State is ' + (data.state ? 'ON' : 'OFF');";
    html += "};";
    html += "</script></body></html>";
    request->send(200, "text/html", html);
}

// Send state update to all WebSocket clients
void publishState() {
    String jsonResponse = "{\"state\": " + String(state ? "true" : "false") + "}";
    ws.textAll(jsonResponse); // Send the JSON response to all connected clients
}

// WebSocket event handler
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WebSocket client disconnected");
    }
}

void setupPeripherals() {
    Serial.begin(115200);
    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.print("Connected to WiFi! IP Address: ");
    Serial.println(WiFi.localIP());

    // WebSocket event handler
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);

    // Serve the webpage
    server.on("/", HTTP_GET, handleRoot);

    server.begin();
}

void loop() {
    // Toggle the state every 2 seconds
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        state = !state; // Toggle state
        publishState(); // Notify WebSocket clients
        Serial.print("Toggled state to: ");
        Serial.println(state ? "ON" : "OFF");
    }
}
