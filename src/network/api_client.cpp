#include "api_client.h"
#include "../core/config.h"
#include "../core/state.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

bool fetchAllTrainTimes() {
    HTTPClient http;
    http.begin(API_BASE_URL);
    
    // Set timeouts to handle slow Lambda responses (default is often too short)
    // Connection timeout: 10 seconds, Read timeout: 15 seconds
    http.setConnectTimeout(10000);  // 10 seconds to establish connection
    http.setTimeout(15000);         // 15 seconds to read response (covers 5s avg + buffer)

    // Check WiFi connection status
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ERROR] WiFi not connected");
        return false;
    }

    int code = http.GET();
    
    if (code <= 0) {
        Serial.print("[ERROR] HTTP request failed (code: ");
        Serial.print(code);
        Serial.println(")");
        http.end();
        return false;
    }
    
    // Check for HTTP error status codes (4xx, 5xx)
    if (code >= 400) {
        Serial.print("[ERROR] HTTP error: ");
        Serial.println(code);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    // Parse the nested JSON structure
    // Use DynamicJsonDocument to avoid stack overflow (allocates on heap instead of stack)
    DynamicJsonDocument doc(6144);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.print("[ERROR] JSON parsing failed: ");
        Serial.println(error.c_str());
        
        // If NoMemory error, provide helpful diagnostics
        if (error == DeserializationError::NoMemory) {
            Serial.print("[ERROR] Response size: ");
            Serial.print(payload.length());
            Serial.println(" bytes, buffer: 6144 bytes");
        }
        
        return false;
    }

    // Reset train times
    aTrainTimes.upCount = 0;
    aTrainTimes.downCount = 0;
    qTrainTimes.upCount = 0;
    qTrainTimes.downCount = 0;
    for (int i = 0; i < 3; i++) {
        aTrainTimes.upTimes[i] = -1;
        aTrainTimes.downTimes[i] = -1;
        qTrainTimes.upTimes[i] = -1;
        qTrainTimes.downTimes[i] = -1;
    }

    // Extract A train from "Jay Street-MetroTech" station
    if (doc.containsKey("Jay Street-MetroTech")) {
        JsonObject jayStation = doc["Jay Street-MetroTech"];
        if (jayStation.containsKey("A")) {
            JsonObject aTrain = jayStation["A"];
            
            // Parse UP times
            if (aTrain.containsKey("UP") && aTrain["UP"].is<JsonArray>()) {
                JsonArray upArray = aTrain["UP"];
                int count = (upArray.size() < 3) ? upArray.size() : 3;
                for (int i = 0; i < count; i++) {
                    aTrainTimes.upTimes[i] = upArray[i].as<int>();
                    aTrainTimes.upCount++;
                }
            }
            
            // Parse DOWN times
            if (aTrain.containsKey("DOWN") && aTrain["DOWN"].is<JsonArray>()) {
                JsonArray downArray = aTrain["DOWN"];
                int count = (downArray.size() < 3) ? downArray.size() : 3;
                for (int i = 0; i < count; i++) {
                    aTrainTimes.downTimes[i] = downArray[i].as<int>();
                    aTrainTimes.downCount++;
                }
            }
        } else {
            Serial.println("[WARNING] 'A' train not found in 'Jay Street-MetroTech' station");
        }
    } else {
        Serial.println("[WARNING] 'Jay Street-MetroTech' station not found in response");
    }

    // Extract Q train from "Dekalb" station
    if (doc.containsKey("Dekalb")) {
        JsonObject dekalbStation = doc["Dekalb"];
        if (dekalbStation.containsKey("Q")) {
            JsonObject qTrain = dekalbStation["Q"];
            
            // Parse UP times
            if (qTrain.containsKey("UP") && qTrain["UP"].is<JsonArray>()) {
                JsonArray upArray = qTrain["UP"];
                int count = (upArray.size() < 3) ? upArray.size() : 3;
                for (int i = 0; i < count; i++) {
                    qTrainTimes.upTimes[i] = upArray[i].as<int>();
                    qTrainTimes.upCount++;
                }
            }
            
            // Parse DOWN times
            if (qTrain.containsKey("DOWN") && qTrain["DOWN"].is<JsonArray>()) {
                JsonArray downArray = qTrain["DOWN"];
                int count = (downArray.size() < 3) ? downArray.size() : 3;
                for (int i = 0; i < count; i++) {
                    qTrainTimes.downTimes[i] = downArray[i].as<int>();
                    qTrainTimes.downCount++;
                }
            }
        }
    }
    
    return true;
}
