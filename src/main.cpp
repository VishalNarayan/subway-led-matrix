#include <Arduino.h>
#include "core/config.h"
#include "core/state.h"
#include "network/api_client.h"
#include "network/wifi_manager.h"
#include "display/display_config.h"
#include "display/display_renderer.h"

MatrixPanel_I2S_DMA* dma_display = nullptr;

void setup() {
    Serial.begin(115200);
    delay(300);

    dma_display = initDisplay();
    setupWiFi(dma_display);
    fetchAllTrainTimes();
    renderDisplay(dma_display);
    lastUpdate = millis();
}

void loop() {
    if (millis() - lastUpdate >= REFRESH_INTERVAL_MS) {
        lastUpdate = millis();
        Serial.println("Refreshing train data...");
        
        if (fetchAllTrainTimes()) {
            renderDisplay(dma_display);
        } else {
            Serial.println("Failed to update data");
        }
    }
}
