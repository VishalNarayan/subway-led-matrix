# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Maintenance Rules

- **CLAUDE.md** and **TODO.md** must be reviewed and updated (if relevant) with every committed change. Keep both files in sync with the actual codebase.

## Project Overview

ESP32-based NYC subway arrival time display. An ESP32 DOIT DevKit V1 fetches real-time train arrival times directly from MTA GTFS-RT protobuf feeds and renders them on a 64x64 HUB75 LED matrix panel. Currently tracks the A train at Jay Street-MetroTech and the Q train at Dekalb.

## Build & Upload Commands

This is a PlatformIO project (not a standard Arduino IDE project).

```bash
pio run                          # Build firmware
pio run -t upload                # Build and flash to ESP32
pio device monitor               # Serial monitor (115200 baud)
pio run -t upload && pio device monitor  # Flash and monitor in one step
```

## Architecture

**Main loop** (`src/main.cpp`): Initializes the display, runs WiFi provisioning (loads NVS credentials or launches captive portal), fetches train data, renders display, then refreshes every 30 seconds.

**Modules:**
- `core/config` — Display dimensions/layout constants and GPIO pin mappings. (Legacy hardcoded WiFi credentials remain in `config.cpp` but are unused; WiFi is now managed via NVS provisioning.)
- `core/state` — Global `TrainTimes` structs holding up to 3 arrival times (minutes) per direction for each train line.
- `network/wifi_manager` — WiFi provisioning: loads credentials from NVS, attempts connection (15s timeout), falls back to captive portal AP mode ("SubwaySign"). Scans for networks and serves a mobile-friendly config page. After connecting, syncs NTP time via `pool.ntp.org` for accurate arrival time calculations. Uses built-in `WebServer`, `DNSServer`, and `Preferences` libraries (no extra `lib_deps`). Shows status on the HUB75 matrix during provisioning using Default 6x8 and Picopixel fonts.
- `network/mta_fetcher` — Fetches MTA GTFS-RT protobuf feeds directly over HTTPS (`WiFiClientSecure` + `HTTPClient`), parses them with nanopb streaming callbacks, and populates `TrainTimes` structs. Feed config (URLs, route IDs, stop IDs) is hardcoded in `feeds[]` array. Uses `setInsecure()` for TLS.
- `network/proto/` — Generated nanopb code from `gtfs-realtime.proto`. The `.options` file constrains field sizes and ignores unused messages to minimize RAM. To regenerate: run the nanopb protoc generator with `--nanopb_opt=-f<path-to-options-file>` (requires Python `protobuf` and `grpcio-tools` packages).
- `display/display_config` — HUB75 matrix initialization via ESP32-HUB75-MatrixPanel-DMA library.
- `display/display_renderer` — Draws two train sections (top/bottom halves of 64x64 display): station label, 10x10 train logo, direction arrows, and right-aligned arrival times.
- `display/draw_utils` — Low-level text drawing helpers.
- `assets/fonts/` — Custom `Font3x5Digits` and `Font5x5Caps` bitmap fonts (Adafruit GFX format).
- `assets/logos/` — Pixel art train line logos in 10x10 and 20x20 sizes, stored as `PROGMEM` uint16_t arrays (RGB565).

**Helper scripts:**
- `scale_logo.py` — Python script to downsample 20x20 logo bitmaps to 10x10.
- `lib/convert.py` — Converts Piskel sprite files to C header arrays. Source `.piskel` files are in `lib/`.

## Key Dependencies

Defined in `platformio.ini`:
- `ESP32-HUB75-MatrixPanel-DMA` — DMA-driven HUB75 LED matrix driver
- `Adafruit GFX Library` — Graphics primitives and font rendering
- `nanopb/Nanopb` — Lightweight protobuf library for parsing MTA GTFS-RT feeds

## Feature Backlog

See `TODO.md` for planned and completed features.

## Display Layout

The 64x64 matrix is split into two 32px-tall sections. Each section shows: a 3-letter station label, a 10x10 train logo, up/down direction arrows, and comma-separated arrival minutes right-aligned to x=55.
