#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>

// On many ESP32 DevKit boards the onboard LED is on GPIO2
const int LED_PIN = 2;
const unsigned long blinkInterval = 500; // Blink interval in milliseconds

WiFiManager wm;

WebServer server(80);
bool serverStarted = false;

void handleResetGet() {
    const char* html = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Reset Wi-Fi</title></head><body><h1>Forget Wi-Fi</h1><p>This will clear saved Wi-Fi credentials from the device.</p><form method=\"POST\" action=\"/reset\"><button type=\"submit\">Forget Wi-Fi and Restart</button></form></body></html>";
    server.send(200, "text/html", html);
}

void handleResetPost() {
    server.send(200, "text/html", "<html><body><h1>Resetting...</h1><p>Device will forget Wi-Fi and restart.</p></body></html>");
    delay(500);
    wm.resetSettings();
    WiFi.disconnect(true);
    delay(500);
    ESP.restart();
}

void startWebServer() {
    if (serverStarted) return;
    server.on("/reset", HTTP_GET, handleResetGet);
    server.on("/reset", HTTP_POST, handleResetPost);
    server.begin();
    serverStarted = true;
    Serial.println("HTTP server started on port 80");
}

void setup() {
    // initialize the LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
    Serial.begin(115200);
    // Print board MAC address at startup
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
    
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  

    wm.setClass("invert"); // dark theme
    wm.setScanDispPerc(true); // display percentages instead of graphs for RSSI

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    wm.resetSettings();

    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    // wm.setConfigPortalBlocking(true); // non blocking config portal

    if(wm.autoConnect("AutoConnectAP", "password")) {
        Serial.println("connected...yeey :)");
        Serial.print("IP address: ");  
        Serial.println(WiFi.localIP());
        // start reset web endpoint
        startWebServer();
    } else {
        Serial.println("Configportal running");
    }

}

void loop() {
    // put your main code here, to run repeatedly: 
    if(WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        
    } else {
        digitalWrite(LED_PIN, HIGH); // turn off LED
        // handle web server clients when connected
        if (serverStarted) server.handleClient();
    }
    delay(1000);
}
