#pragma once

// Train times storage structure
struct TrainTimes {
    int upTimes[3] = {-1, -1, -1};
    int upCount = 0;
    int downTimes[3] = {-1, -1, -1};
    int downCount = 0;
};

// Global state
extern TrainTimes aTrainTimes;
extern TrainTimes qTrainTimes;
extern unsigned long lastUpdate;
