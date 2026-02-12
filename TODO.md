# Feature Backlog

## ~~1. WiFi Provisioning via Phone~~ ✅ DONE
Implemented captive portal WiFi provisioning. On first boot (or when saved credentials fail), the ESP32 enters AP mode as "SubwaySign", scans for nearby networks, and serves a mobile-friendly configuration page. Users select their network from a dropdown and enter their password. Credentials are saved to NVS and persist across reboots. See `src/network/wifi_manager.cpp`.

## 2. Configurable Stations and Train Lines
Currently hardcoded to A train at Jay Street-MetroTech and Q train at Dekalb. The system should support arbitrary stations, train lines, and directions — not just two. This ties into the phone-based configuration UI (item 4). Data structures in `core/state` and feed config in `network/mta_fetcher` will need to become dynamic rather than fixed to two trains.

## ~~3. Move MTA Data Fetching into Firmware~~ ✅ DONE
The ESP32 now fetches MTA GTFS-RT protobuf feeds directly over HTTPS and parses them on-device using nanopb. NTP time sync provides accurate "minutes from now" calculations. The AWS Lambda proxy and ArduinoJson dependency have been removed. See `src/network/mta_fetcher.cpp` and `src/network/proto/`.

## 4. Local Web UI for Configuration
Host a web server on the ESP32 (over the local WiFi network) that serves a page accessible from the user's phone. The web UI should let users select which stations, train lines, and directions to display. Configuration should be saved to NVS/EEPROM. This replaces any need for hardcoded station/train config and works together with items 1 and 2.
