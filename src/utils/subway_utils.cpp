#include "subway_utils.h"
#include <ArduinoJson.h>

/*
 * Parse a simple JSON array and return the first 3 numbers
 * Example: [7, 8, 14, 25, 36, 49, 59, 85, 96]
 * Returns: out[0]=7, out[1]=8, out[2]=14, outCount=3 (or outCount=0 if parsing fails)
 */
void parseTrainTimes(const String& jsonResponse, int out[3], int& outCount) {
    outCount = 0;
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }
    
    // Check if it's a JSON array
    if (doc.is<JsonArray>()) {
        JsonArray arr = doc.as<JsonArray>();
        int count = (arr.size() < 3) ? arr.size() : 3;
        for (int i = 0; i < count; i++) {
            out[i] = arr[i].as<int>();
            outCount++;
        }
    }
}
