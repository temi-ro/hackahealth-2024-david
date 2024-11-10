#include <WiFi.h>
#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include "peripherals.h"

const char* ssid = "";
const char* password = "";
constexpr long BAUD_RATE = 115200;
constexpr long TOGGLE_INTERVAL = 5000;

// const char* ssid = "ESP32-Access-Point";
// const char* password = "123456789";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); 

unsigned long previousMillis = 0;   // Tracks the last time the state was toggled
const long interval = 5000;         // Toggle interval (2 seconds)

enum State {
    IDLE,
    REQUESTED,
    COMING,
    ERROR
};

State state = IDLE;
const String stateToString(State state){
    switch (state){
        case IDLE:
            return "IDLE";
        case REQUESTED:
            return "REQUESTED";
        case COMING:
            return "COMING";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

/***** Function prototypes *****/ 
void publishState();
void switchState(State newState);
void requestService();
void handleRoot(AsyncWebServerRequest *request);
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void notifyClients();
/* ************************** */

void setup() {
    Serial.begin(BAUD_RATE);


    // Set your Static IP address
    IPAddress local_IP(192, 168, 43, 184);
    // Set your Gateway IP address
    IPAddress gateway(192, 168, 43, 1);

    IPAddress subnet(255, 255, 0, 0);
    IPAddress primaryDNS(8, 8, 8, 8);   //optional
    IPAddress secondaryDNS(8, 8, 4, 4); //optional

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("STA Failed to configure");
    }

    setupPeripherals();
    // WiFi.softAP(ssid, password);
    // delay(4000);
    
    WiFi.begin(ssid, password);
    delay(2000);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.print("Connected to WiFi! IP Address: ");
    Serial.println(WiFi.localIP());

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS file system");
        return;
    }

    // WebSocket event handler
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);

    // Serve the webpage
    server.on("/", HTTP_GET, handleRoot);

    server.on("/sound-coming.wav", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/sound-coming.wav", "audio/wav");
    });

    server.on("/sound-requested.wav", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/sound-requested.wav", "audio/wav");
    });

    server.begin();
}

void loop() {
    loopPheripherals();
    // Handle snooze button press
    if (snoozePressed){
        switchState(COMING);
        snoozePressed = false;
    }
    // Toggle the state every 2 seconds
    // unsigned long currentMillis = millis();
    // if (currentMillis - previousMillis >= TOGGLE_INTERVAL) {
    //     previousMillis = currentMillis;
    //     if (state == REQUESTED){
    //         switchState(COMING);
    //     }
    // }
}

void switchState(State newState){
    state = newState;
    Serial.print("State changed to: " + stateToString(state) + "\n");
    publishState();
}

// Send state update to all WebSocket clients
void publishState() {
    String msg = "{\"state\": \"" + stateToString(state) + "\"}";
    ws.textAll(msg);
}

// Serve the webpage with WebSocket support
void handleRoot(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
}

// Send state update to all WebSocket clients
void notifyClients() {
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
    } else if (type == WS_EVT_DATA) {
        // A message was received from the client
        String msg = String((char*)data);  // Convert the data to a string
        Serial.print("Message received: ");
        Serial.println(msg);

        if (msg.indexOf("request") != -1){
            switchState(REQUESTED);
            requestService();
        }else if (msg.indexOf("idle") != -1){
            switchState(IDLE);
        }else if (msg.indexOf("cancel") != -1){
            switchState(IDLE);
            handleCancelPress();
        }
        // Respond with an acknowledgment message
        // String response = "Received: " + msg;
        // client->text(response);  // Send the response back to the client
    }
}

void requestService() {
    Serial.println("Service requested! Currently in state: " + stateToString(state));
    #ifndef BTN_DAVID
        handleServiceRequestPress();
    #endif
}

