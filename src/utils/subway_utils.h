#pragma once

#include <Arduino.h>

void parseTrainTimes(const String& jsonResponse, int out[3], int& outCount);