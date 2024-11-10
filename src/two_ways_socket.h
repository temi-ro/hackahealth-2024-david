#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* SSID = "DarkSideOfTheMoon";
const char* PASSWORD = "maytheforcebewithyou";
constexpr long TOGGLE_INTERVAL = 2000;        
constexpr int CODE_OK = 200;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket endpoint
bool state = false;
unsigned long previousMillis = 0;   // Tracks the last time the state was toggled


// Serve the webpage with WebSocket support
void handleRoot(AsyncWebServerRequest *request) {
    String html = "";

    // Define style
    html += "<!DOCTYPE html><html><head><style>";
    html += ".button { padding: 15px; font-size: 16px; }";
    html += ".on { background-color: green; color: white; }";
    html += ".off { background-color: red; color: white; }";
    html += "</style></head><body>";

    // Title
    html += "<h1>ESP32 State Button</h1>";

    // Button to display state
    html += "<button id='stateButton' class='button ";
    html += (state ? "on" : "off");
    html += "'>State is ";
    html += (state ? "ON" : "OFF");
    html += "</button><br><br>";

    // Button to send "hello" message
    html += "<button id='helloButton' class='button'>Send Hello</button>";

    // JavaScript to handle WebSocket messages
    html += "<script>";
    html += "var socket = new WebSocket('ws://' + location.host + '/ws');";
    html += "socket.onmessage = function(event) {";
    html += "var data = JSON.parse(event.data);";
    html += "document.getElementById('stateButton').className = 'button ' + (data.state ? 'on' : 'off');";
    html += "document.getElementById('stateButton').innerText = 'State is ' + (data.state ? 'ON' : 'OFF');";
    html += "};";

    // JavaScript to send "hello" message when helloButton is clicked
    html += "document.getElementById('helloButton').onclick = function() {";
    html += "socket.send(JSON.stringify({ command: 'hello' }));";
    html += "};";
    html += "</script></body></html>";

    // Send the HTML response
    request->send(CODE_OK, "text/html", html);
}


// Send state update to all WebSocket clients
void publishState() {
    String jsonResponse = "{\"state\": " + String(state ? "true" : "false") + "}";
    ws.textAll(jsonResponse); // Send the JSON response to all connected clients
}


// WebSocket event handler
void onWebSocketEvent(
    AsyncWebSocket *server, 
    AsyncWebSocketClient *client, 
    AwsEventType type,
    void *arg, 
    uint8_t *data, 
    size_t len
) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
        // Handle incoming WebSocket messages
        String message = String((char*)data).substring(0, len);
        if (message.indexOf("\"command\":\"hello\"") != -1) {
            Serial.println("Hello");
        }
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
    if (currentMillis - previousMillis >= TOGGLE_INTERVAL) {
        previousMillis = currentMillis;
        state = !state; // Toggle state
        publishState(); // Notify WebSocket clients
    }
}
