#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Handles full WiFi setup: loads NVS credentials, attempts connection,
// falls back to captive portal AP mode if needed.
// Blocks until WiFi is connected (may restart ESP on portal save).
void setupWiFi(MatrixPanel_I2S_DMA* display);

// Clears saved WiFi credentials from NVS
void clearSavedCredentials();
