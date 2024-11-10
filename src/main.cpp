#include <WiFi.h>
#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include "peripherals.h"

constexpr unsigned long BAUD_RATE = 115200;
constexpr unsigned long PORT = 80;

Preferences preferences;
String ssid;
String password;
String staticIpStr;
AsyncWebServer server(PORT);
AsyncWebSocket ws("/ws"); 

enum State {
    IDLE,
    REQUESTED,
    COMING,
    ERROR
};

State state = IDLE;

const String stateToString(State state) {
    switch (state) {
        case IDLE: return "IDLE";
        case REQUESTED: return "REQUESTED";
        case COMING: return "COMING";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/***** Function prototypes *****/ 
void switchState(State newState);
void requestService();
void handleRoot(AsyncWebServerRequest *request);
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void notifyClients();
void connectToWiFi(unsigned int numberOfAttempts=20);
void loadWiFiCredentials();
void saveWiFiCredentials();
void handleSerialCommands();
/* *************************** */

void setup() {
    Serial.begin(BAUD_RATE);

    preferences.begin("wifi", false);
    loadWiFiCredentials();

    setupPeripherals();
    connectToWiFi();

    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS file system");
        return;
    }

    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);

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
    updatePeripherals();
    handleSerialCommands();

    if (snoozePressed){
        switchState(COMING);
        snoozePressed = false;
    }
}

void switchState(State newState){
    state = newState;
    Serial.print("State changed to: " + stateToString(state) + "\n");
    String msg = "{\"state\": \"" + stateToString(state) + "\"}";
    ws.textAll(msg);
}

void handleRoot(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
}

void notifyClients() {
    String jsonResponse = "{\"state\": " + String(state ? "true" : "false") + "}";
    ws.textAll(jsonResponse);
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
        String msg = String((char*)data);
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
    }
}

void requestService() {
    Serial.println("Service requested! Currently in state: " + stateToString(state));
    #ifndef BTN_DAVID
        handleServiceRequestPress();
    #endif
}

void connectToWiFi(unsigned int numberOfAttempts) {
    if (!staticIpStr.isEmpty()) {
        IPAddress local_IP, gateway, subnet(255, 255, 255, 0);
        IPAddress primaryDNS(8, 8, 8, 8);
        IPAddress secondaryDNS(8, 8, 4, 4);

        if (local_IP.fromString(staticIpStr)) {
            gateway = local_IP;
            gateway[3] = 1; // Set last segment to 1 for gateway
            if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
                Serial.println("Failed to configure WiFi");
            } else {
                Serial.println("Static IP configured.");
            }
        } else {
            Serial.println("Invalid static IP format. Using DHCP.");
        }
    } else {
        Serial.println("No static IP found. Using DHCP.");
    }

    WiFi.begin(ssid.c_str(), password.c_str());

    for (unsigned int attempt = 1; attempt <= numberOfAttempts; attempt++) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("Connected to WiFi! IP Address: ");
            Serial.println(WiFi.localIP());
            return;
        }

        Serial.print("WiFi connection attempt ");
        Serial.print(attempt);
        Serial.print("/");
        Serial.print(numberOfAttempts);
        Serial.println(" failed. Retrying...");
        delay(1000);
    }

    Serial.println("Failed to connect to WiFi after maximum attempts.");
}


void loadWiFiCredentials() {
    preferences.begin("wifi", true);
    ssid = preferences.getString("ssid", "admin");
    password = preferences.getString("password", "1234");
    staticIpStr = preferences.getString("static_ip", "");
    preferences.end();
}

void saveWiFiCredentials() {
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("static_ip", staticIpStr);
    preferences.end();
    Serial.println("WiFi credentials saved to non-volatile memory.");
}

void handleSerialCommands() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.equals("help")) {
            Serial.println("Available commands:");
            Serial.println("  help                          - Show available commands");
            Serial.println("  info                          - Show connection info");
            Serial.println("  set_network <network_name>    - Set WiFi network name. Prompts for password.");
            Serial.println("  set_ip <ip_address>           - Set a static IP address for the server.");
            delay(3000);
        } 
        else if (command.equals("info")) {
            Serial.println("Connection Information:");
            Serial.print("  SSID: ");
            Serial.println(ssid);
            Serial.print("  IP Address: ");
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println(WiFi.localIP());
            } else {
                Serial.println("Not connected");
            }
            Serial.print("  Static IP: ");
            Serial.println(staticIpStr.isEmpty() ? "Not configured" : staticIpStr);
            Serial.print("  Connection Status: ");
            Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
        }
        else if (command.startsWith("set_network ")) {
            ssid = command.substring(12);
            Serial.print("Enter password for ");
            Serial.print(ssid);
            Serial.print(": ");
            
            while (!Serial.available()) {}
            password = Serial.readStringUntil('\n');
            password.trim();
            
            Serial.print("Connecting to new network: ");
            Serial.println(ssid);
            saveWiFiCredentials();
            connectToWiFi();
        }
        else if (command.startsWith("set_ip ")) {
            staticIpStr = command.substring(7);
            Serial.print("Setting static IP to ");
            Serial.println(staticIpStr);
            saveWiFiCredentials();
            connectToWiFi();
        }
    }
}
