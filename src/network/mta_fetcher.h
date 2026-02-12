#pragma once

#include "../core/state.h"

// Fetch train times directly from MTA GTFS-RT protobuf feeds.
// Populates aTrainTimes and qTrainTimes globals.
// Returns true if at least one feed was parsed successfully.
bool fetchTrainTimes();
