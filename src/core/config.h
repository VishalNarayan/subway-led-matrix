#pragma once

// WiFi Configuration
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// API Configuration
extern const char* API_BASE_URL;

// Display Configuration
const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 64;
const int DISPLAY_BRIGHTNESS = 100;
const int DISPLAY_GPIO_B = 22;
const int DISPLAY_GPIO_E = 32;

// Refresh Configuration
const unsigned long REFRESH_INTERVAL_MS = 30000;  // 30 seconds

// Display Layout Constants
const int LOGO_SIZE = 10;  // 10x10 logos
const int SECTION_HEIGHT = 32;
const int RIGHT_EDGE_X = 55;
const int TEXT_Y_OFFSET = 6;
const int LOGO_Y_OFFSET = 8;
const int ARROW_UP = 91;   // ASCII code for up arrow
const int ARROW_DOWN = 92; // ASCII code for down arrow
