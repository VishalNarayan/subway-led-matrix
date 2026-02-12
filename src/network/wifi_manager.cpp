#include "wifi_manager.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Fonts/Picopixel.h>

static const char* NVS_NAMESPACE = "subway-wifi";
static const char* NVS_KEY_SSID = "ssid";
static const char* NVS_KEY_PASS = "password";

static const char* AP_SSID = "SubwaySign";
static const unsigned long CONNECT_TIMEOUT_MS = 15000;
static const byte DNS_PORT = 53;

// HTML template — %NETWORKS% placeholder is replaced with scanned <option> tags
static const char PORTAL_HTML_START[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SubwaySign WiFi Setup</title>
<style>
body{font-family:sans-serif;background:#1a1a2e;color:#e0e0e0;margin:0;
display:flex;justify-content:center;align-items:center;min-height:100vh}
.card{background:#16213e;padding:24px;border-radius:12px;width:280px;
box-shadow:0 4px 12px rgba(0,0,0,0.4)}
h2{margin:0 0 16px;text-align:center;color:#fff}
label{display:block;margin:12px 0 4px;font-size:14px}
select,input[type=password],input[type=text]{width:100%;padding:10px;
border:1px solid #333;border-radius:6px;background:#0f3460;color:#fff;
font-size:16px;box-sizing:border-box;-webkit-appearance:none}
.pass-wrap{position:relative}
.pass-wrap input{width:100%;padding-right:44px}
.toggle{position:absolute;right:8px;top:50%;transform:translateY(-50%);
background:none;border:none;color:#888;font-size:13px;padding:4px;
width:auto;margin:0;cursor:pointer}
button[type=submit]{width:100%;padding:12px;margin-top:20px;border:none;
border-radius:6px;background:#e94560;color:#fff;font-size:16px;
cursor:pointer}
button[type=submit]:hover{background:#c73a52}
.info{text-align:center;font-size:12px;color:#888;margin-top:12px}
</style>
</head>
<body>
<div class="card">
<h2>SubwaySign</h2>
<form action="/save" method="POST">
<label for="ssid">WiFi Network</label>
<select id="ssid" name="ssid" required>
<option value="">Select a network...</option>
)rawliteral";

static const char PORTAL_HTML_END[] PROGMEM = R"rawliteral(
</select>
<label for="pass">Password</label>
<div class="pass-wrap">
<input type="password" id="pass" name="pass">
<button type="button" class="toggle" onclick="var p=document.getElementById('pass');if(p.type==='password'){p.type='text';this.textContent='Hide';}else{p.type='password';this.textContent='Show';}">Show</button>
</div>
<button type="submit">Connect</button>
</form>
<div class="info">Device will restart after saving.</div>
</div>
</body>
</html>
)rawliteral";

// Scan for WiFi networks and return HTML <option> tags
static String scanNetworksHTML() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int n = WiFi.scanNetworks();
    Serial.printf("Scan found %d networks\n", n);

    String options;
    // Track seen SSIDs to avoid duplicates
    String seen[32];
    int seenCount = 0;

    for (int i = 0; i < n && seenCount < 32; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) continue;

        // Skip duplicates
        bool dup = false;
        for (int j = 0; j < seenCount; j++) {
            if (seen[j] == ssid) { dup = true; break; }
        }
        if (dup) continue;
        seen[seenCount++] = ssid;

        int rssi = WiFi.RSSI(i);
        const char* signal = rssi > -50 ? "Strong" : rssi > -70 ? "Good" : "Weak";
        options += "<option value=\"" + ssid + "\">" + ssid + " (" + signal + ")</option>\n";
    }

    WiFi.scanDelete();
    return options;
}

static const char SAVED_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Saved</title>
<style>
body{font-family:sans-serif;background:#1a1a2e;color:#e0e0e0;
display:flex;justify-content:center;align-items:center;min-height:100vh}
.card{background:#16213e;padding:24px;border-radius:12px;text-align:center}
h2{color:#4ecca3}
</style>
</head>
<body>
<div class="card">
<h2>Saved!</h2>
<p>Restarting device...</p>
</div>
</body>
</html>
)rawliteral";

// Show a status message on the matrix display
static void showStatus(MatrixPanel_I2S_DMA* display, const char* lines[], int count) {
    display->clearScreen();
    display->setFont(NULL);  // default 6x8 Adafruit GFX font
    display->setTextSize(1);
    display->setTextColor(display->color444(15, 15, 15));

    int totalHeight = count * 10;
    int startY = (64 - totalHeight) / 2;

    for (int i = 0; i < count; i++) {
        int textWidth = strlen(lines[i]) * 6;
        int x = (64 - textWidth) / 2;
        display->setCursor(x, startY + i * 10);
        display->print(lines[i]);
    }
}

static bool tryConnect(MatrixPanel_I2S_DMA* display, const String& ssid, const String& password) {
    Serial.printf("Connecting to WiFi: %s\n", ssid.c_str());

    // Show "Connecting to" in default font, SSID in smaller Picopixel font
    display->clearScreen();
    display->setFont(NULL);
    display->setTextSize(1);
    display->setTextColor(display->color444(15, 15, 15));

    const char* label = "Connecting";
    int labelW = strlen(label) * 6;
    display->setCursor((64 - labelW) / 2, 22);
    display->print(label);

    // Switch to Picopixel (4px per char) for the SSID
    display->setFont(&Picopixel);
    int ssidW = ssid.length() * 4;
    display->setCursor((64 - ssidW) / 2, 38);
    display->print(ssid);
    display->setFont(NULL);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < CONNECT_TIMEOUT_MS) {
        delay(300);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        const char* connectedLines[] = {"Connected!"};
        showStatus(display, connectedLines, 1);
        delay(1500);
        return true;
    }

    Serial.println("Connection failed.");
    WiFi.disconnect();
    return false;
}

static volatile bool credentialsSaved = false;

static void runCaptivePortal(MatrixPanel_I2S_DMA* display) {
    Serial.println("Scanning for networks...");

    // Picopixel, all caps, centered
    display->clearScreen();
    display->setFont(&Picopixel);
    display->setTextSize(1);
    display->setTextColor(display->color444(15, 15, 15));
    const char* s1 = "SCANNING";
    const char* s2 = "NETWORKS...";
    display->setCursor((64 - strlen(s1) * 4) / 2, 28);
    display->print(s1);
    display->setCursor((64 - strlen(s2) * 4) / 2, 38);
    display->print(s2);
    display->setFont(NULL);

    String networkOptions = scanNetworksHTML();

    credentialsSaved = false;

    const char* apLines[] = {"WiFi Setup", "Connect to", "SubwaySign", "on phone"};
    showStatus(display, apLines, 4);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID);
    delay(100);
    Serial.printf("AP started. IP: %s\n", WiFi.softAPIP().toString().c_str());

    DNSServer* dnsServer = new DNSServer();
    WebServer* webServer = new WebServer(80);

    dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

    webServer->on("/", HTTP_GET, [webServer, networkOptions]() {
        String html = FPSTR(PORTAL_HTML_START);
        html += networkOptions;
        html += FPSTR(PORTAL_HTML_END);
        webServer->send(200, "text/html", html);
    });

    webServer->on("/save", HTTP_POST, [webServer]() {
        String ssid = webServer->arg("ssid");
        String pass = webServer->arg("pass");

        if (ssid.length() > 0) {
            Preferences prefs;
            prefs.begin(NVS_NAMESPACE, false);
            prefs.putString(NVS_KEY_SSID, ssid);
            prefs.putString(NVS_KEY_PASS, pass);
            prefs.end();
            Serial.printf("Saved credentials for: %s\n", ssid.c_str());
            credentialsSaved = true;
        }

        webServer->send_P(200, "text/html", SAVED_HTML);
    });

    // Redirect all unknown paths to portal (captive portal detection)
    webServer->onNotFound([webServer]() {
        webServer->sendHeader("Location", "http://192.168.4.1/", true);
        webServer->send(302, "text/plain", "");
    });

    webServer->begin();
    Serial.println("Portal server started.");

    // Run portal loop until credentials are saved via /save handler
    while (!credentialsSaved) {
        dnsServer->processNextRequest();
        webServer->handleClient();
        delay(10);
    }

    // Let the HTTP response finish sending
    delay(1500);

    webServer->stop();
    dnsServer->stop();
    delete webServer;
    delete dnsServer;

    const char* restartLines[] = {"Restarting", "..."};
    showStatus(display, restartLines, 2);
    delay(1000);

    ESP.restart();
}

void setupWiFi(MatrixPanel_I2S_DMA* display) {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    String savedSSID = prefs.getString(NVS_KEY_SSID, "");
    String savedPass = prefs.getString(NVS_KEY_PASS, "");
    prefs.end();

    if (savedSSID.length() > 0) {
        Serial.printf("Found saved credentials for: %s\n", savedSSID.c_str());
        if (tryConnect(display, savedSSID, savedPass)) {
            return;
        }
        Serial.println("Saved credentials failed, starting portal...");
    } else {
        Serial.println("No saved WiFi credentials found.");
    }

    runCaptivePortal(display);
    // runCaptivePortal calls ESP.restart(), so we never return here on portal path
}

void clearSavedCredentials() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);
    prefs.clear();
    prefs.end();
    Serial.println("WiFi credentials cleared from NVS.");
}
