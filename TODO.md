# Feature Backlog

## Completed

### ~~1. WiFi Provisioning via Phone~~ ✅
Implemented captive portal WiFi provisioning. On first boot (or when saved credentials fail), the ESP32 enters AP mode as "SubwaySign", scans for nearby networks, and serves a mobile-friendly configuration page. Users select their network from a dropdown and enter their password. Credentials are saved to NVS and persist across reboots. See `src/network/wifi_manager.cpp`.

### ~~3. Move MTA Data Fetching into Firmware~~ ✅
The ESP32 now fetches MTA GTFS-RT protobuf feeds directly over HTTPS and parses them on-device using nanopb. NTP time sync provides accurate "minutes from now" calculations. The AWS Lambda proxy and ArduinoJson dependency have been removed. See `src/network/mta_fetcher.cpp` and `src/network/proto/`.

---

## Configuration

### 2. Configurable Stations and Train Lines
Currently hardcoded to A train at Jay Street-MetroTech and Q train at Dekalb. The system should support arbitrary stations, train lines, and directions — not just two. This ties into the local web UI (item 4). Data structures in `core/state` and feed config in `network/mta_fetcher` will need to become dynamic rather than fixed to two trains.

### 4. Local Web UI for Configuration
Host a web server on the ESP32 (over the local WiFi network) that serves a page accessible from the user's phone. The web UI should let users select which stations, train lines, and directions to display. Configuration should be saved to NVS/EEPROM. This replaces any need for hardcoded station/train config and works together with items 1 and 2.

## Display

### 5. Redesign Display UI
Rethink the 64x64 matrix layout. Currently shows two train lines in two 32px-tall sections. Explore showing more than two lines, better use of space, and improved readability.

### 6. Night Mode (Red Light)
Add a red-tinted low-brightness display mode for nighttime use. Could be time-based (automatic) or toggled via the web UI.

### 7. Weather Mode
Add an alternate display mode that shows current weather conditions. Could cycle with train times or be selectable via the web UI.

## Data & Reliability

### 8. Increase Refresh Frequency
Currently refreshes every 30 seconds. Evaluate faster refresh intervals for more up-to-date arrival times.

### 9. Test All Subway Stations and Logos
Verify that all subway line logos render correctly at 10x10 on the matrix and that the system handles all valid station/line/direction combinations. Ensure edge cases (e.g., express-only stops, shuttle lines) are covered.
