#include "mta_fetcher.h"
#include "../core/state.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include <pb_decode.h>
#include "proto/gtfs-realtime.pb.h"

// --- Feed configuration ---
// MTA GTFS-RT endpoint pattern
static const char* MTA_BASE_URL = "https://api-endpoint.mta.info/Dataservice/mtagtfsfeeds/nyct%2Fgtfs";

struct FeedConfig {
    const char* urlSuffix;   // appended to MTA_BASE_URL (e.g., "-ace")
    const char* routeId;     // e.g., "A" or "Q"
    const char* stopUp;      // northbound stop_id
    const char* stopDown;    // southbound stop_id
    TrainTimes* dest;        // where to store results
};

static FeedConfig feeds[] = {
    { "-ace", "A", "A41N", "A41S", &aTrainTimes },
    { "-nqrw", "Q", "R30N", "R30S", &qTrainTimes },
};
static const int NUM_FEEDS = sizeof(feeds) / sizeof(feeds[0]);

// --- Timestamp collection context passed through nanopb callbacks ---
static const int MAX_TIMESTAMPS = 50;

struct ParseContext {
    const char* routeId;
    const char* stopUp;
    const char* stopDown;
    time_t upTimestamps[MAX_TIMESTAMPS];
    int upCount;
    time_t downTimestamps[MAX_TIMESTAMPS];
    int downCount;
};

// --- nanopb stream adapter: read from WiFiClient ---

struct StreamState {
    WiFiClient* client;
    int remaining;
};

static bool stream_read_callback(pb_istream_t* stream, pb_byte_t* buf, size_t count) {
    StreamState* state = (StreamState*)stream->state;

    if (count == 0) return true;
    if (state->remaining <= 0) {
        stream->bytes_left = 0;
        return false;
    }

    size_t toRead = count;
    if ((int)toRead > state->remaining) toRead = state->remaining;

    size_t totalRead = 0;
    unsigned long timeout = millis() + 10000;
    while (totalRead < toRead && millis() < timeout) {
        int avail = state->client->available();
        if (avail > 0) {
            int chunk = state->client->read(buf + totalRead, toRead - totalRead);
            if (chunk > 0) {
                totalRead += chunk;
                timeout = millis() + 10000;  // reset timeout on progress
            }
        } else if (!state->client->connected()) {
            break;
        } else {
            delay(1);
        }
    }

    state->remaining -= totalRead;

    if (totalRead < count) {
        stream->bytes_left = 0;
        return false;
    }
    return true;
}

static pb_istream_t pb_istream_from_wifi(StreamState* state) {
    pb_istream_t stream = {&stream_read_callback, state, (size_t)state->remaining, NULL};
    return stream;
}

// --- nanopb callback: decode each StopTimeUpdate in a TripUpdate ---

static bool stop_time_update_callback(pb_istream_t* stream, const pb_field_t* field, void** arg) {
    ParseContext* ctx = (ParseContext*)*arg;

    transit_realtime_TripUpdate_StopTimeUpdate stu =
        transit_realtime_TripUpdate_StopTimeUpdate_init_default;

    if (!pb_decode(stream, transit_realtime_TripUpdate_StopTimeUpdate_fields, &stu))
        return false;

    if (!stu.has_stop_id) return true;

    // Prefer departure time, fall back to arrival
    int64_t ts = 0;
    if (stu.has_departure && stu.departure.has_time) {
        ts = stu.departure.time;
    } else if (stu.has_arrival && stu.arrival.has_time) {
        ts = stu.arrival.time;
    }
    if (ts == 0) return true;

    if (strcmp(stu.stop_id, ctx->stopUp) == 0) {
        if (ctx->upCount < MAX_TIMESTAMPS) {
            ctx->upTimestamps[ctx->upCount++] = (time_t)ts;
        }
    } else if (strcmp(stu.stop_id, ctx->stopDown) == 0) {
        if (ctx->downCount < MAX_TIMESTAMPS) {
            ctx->downTimestamps[ctx->downCount++] = (time_t)ts;
        }
    }

    return true;
}

// --- nanopb callback: decode each FeedEntity in a FeedMessage ---

static bool entity_callback(pb_istream_t* stream, const pb_field_t* field, void** arg) {
    ParseContext* ctx = (ParseContext*)*arg;

    // Save timestamp counts before decoding this entity.
    // The stop_time_update callback fires DURING pb_decode (before we can
    // check route_id), so we collect timestamps optimistically and roll back
    // if the route doesn't match.
    int savedUpCount = ctx->upCount;
    int savedDownCount = ctx->downCount;

    transit_realtime_FeedEntity entity = transit_realtime_FeedEntity_init_default;

    // Wire up the stop_time_update callback inside the trip_update
    entity.trip_update.stop_time_update.funcs.decode = stop_time_update_callback;
    entity.trip_update.stop_time_update.arg = ctx;

    if (!pb_decode(stream, transit_realtime_FeedEntity_fields, &entity))
        return false;

    // After decoding, check route_id; discard timestamps if no match
    if (!entity.has_trip_update || !entity.trip_update.trip.has_route_id ||
        strcmp(entity.trip_update.trip.route_id, ctx->routeId) != 0) {
        ctx->upCount = savedUpCount;
        ctx->downCount = savedDownCount;
    }

    return true;
}

// --- Sort helper ---
static int cmp_time(const void* a, const void* b) {
    time_t ta = *(const time_t*)a;
    time_t tb = *(const time_t*)b;
    return (ta > tb) - (ta < tb);
}

// --- Convert sorted timestamps to minutes-from-now, store in TrainTimes ---
static void timestamps_to_minutes(time_t* timestamps, int count, int* dest, int* destCount) {
    time_t now = time(nullptr);
    *destCount = 0;

    qsort(timestamps, count, sizeof(time_t), cmp_time);

    for (int i = 0; i < count && *destCount < 3; i++) {
        int mins = (int)((timestamps[i] - now) / 60);
        if (mins < 0) continue;  // already departed
        dest[*destCount] = mins;
        (*destCount)++;
    }

    // Fill remaining slots with -1
    for (int i = *destCount; i < 3; i++) {
        dest[i] = -1;
    }
}

// --- Fetch and parse a single GTFS-RT feed ---
static bool fetchFeed(const FeedConfig& feed) {
    String url = String(MTA_BASE_URL) + feed.urlSuffix;
    Serial.printf("Fetching feed: %s\n", url.c_str());

    WiFiClientSecure client;
    client.setInsecure();  // skip cert verification for now

    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(15000);

    if (!http.begin(client, url)) {
        Serial.println("[MTA] HTTP begin failed");
        return false;
    }

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.printf("[MTA] HTTP error: %d\n", code);
        http.end();
        return false;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
        // Chunked transfer — read into buffer as fallback
        Serial.println("[MTA] No content-length, reading full response");
        WiFiClient* rawStream = http.getStreamPtr();
        // Read up to 256KB
        const int MAX_BUF = 256 * 1024;
        uint8_t* buf = (uint8_t*)malloc(MAX_BUF);
        if (!buf) {
            Serial.println("[MTA] malloc failed");
            http.end();
            return false;
        }
        int total = 0;
        unsigned long timeout = millis() + 15000;
        while (rawStream->connected() && total < MAX_BUF && millis() < timeout) {
            int avail = rawStream->available();
            if (avail > 0) {
                int r = rawStream->read(buf + total, min(avail, MAX_BUF - total));
                if (r > 0) total += r;
            } else {
                delay(1);
            }
        }
        http.end();

        Serial.printf("[MTA] Read %d bytes (buffered)\n", total);

        // Parse from buffer
        ParseContext ctx = {};
        ctx.routeId = feed.routeId;
        ctx.stopUp = feed.stopUp;
        ctx.stopDown = feed.stopDown;

        transit_realtime_FeedMessage msg = transit_realtime_FeedMessage_init_default;
        msg.entity.funcs.decode = entity_callback;
        msg.entity.arg = &ctx;

        pb_istream_t pbStream = pb_istream_from_buffer(buf, total);
        bool ok = pb_decode(&pbStream, transit_realtime_FeedMessage_fields, &msg);
        free(buf);

        if (!ok) {
            Serial.printf("[MTA] Protobuf decode error: %s\n", PB_GET_ERROR(&pbStream));
            return false;
        }

        timestamps_to_minutes(ctx.upTimestamps, ctx.upCount,
                              feed.dest->upTimes, &feed.dest->upCount);
        timestamps_to_minutes(ctx.downTimestamps, ctx.downCount,
                              feed.dest->downTimes, &feed.dest->downCount);
        return true;
    }

    // Known content-length — stream directly
    Serial.printf("[MTA] Content-Length: %d\n", contentLength);
    WiFiClient* rawStream = http.getStreamPtr();

    ParseContext ctx = {};
    ctx.routeId = feed.routeId;
    ctx.stopUp = feed.stopUp;
    ctx.stopDown = feed.stopDown;

    StreamState state = { rawStream, contentLength };
    pb_istream_t pbStream = pb_istream_from_wifi(&state);

    transit_realtime_FeedMessage msg = transit_realtime_FeedMessage_init_default;
    msg.entity.funcs.decode = entity_callback;
    msg.entity.arg = &ctx;

    bool ok = pb_decode(&pbStream, transit_realtime_FeedMessage_fields, &msg);
    http.end();

    if (!ok) {
        Serial.printf("[MTA] Protobuf decode error: %s\n", PB_GET_ERROR(&pbStream));
        return false;
    }

    Serial.printf("[MTA] Parsed %s: %d up, %d down timestamps\n",
                  feed.routeId, ctx.upCount, ctx.downCount);

    timestamps_to_minutes(ctx.upTimestamps, ctx.upCount,
                          feed.dest->upTimes, &feed.dest->upCount);
    timestamps_to_minutes(ctx.downTimestamps, ctx.downCount,
                          feed.dest->downTimes, &feed.dest->downCount);

    return true;
}

// --- Public API ---

bool fetchTrainTimes() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[MTA] WiFi not connected");
        return false;
    }

    // Reset all times
    aTrainTimes = TrainTimes();
    qTrainTimes = TrainTimes();

    bool anySuccess = false;
    for (int i = 0; i < NUM_FEEDS; i++) {
        if (fetchFeed(feeds[i])) {
            anySuccess = true;
        }
    }

    return anySuccess;
}
