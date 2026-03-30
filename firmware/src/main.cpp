/*
Modified FlightWall Firmware
- Optimized for <$5 AeroAPI usage.
- 25-second per-flight display rotation.
*/
#include <vector>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "config/UserConfiguration.h"
#include "config/WiFiConfiguration.h"
#include "config/TimingConfiguration.h"
#include "adapters/OpenSkyFetcher.h"
#include "adapters/AeroAPIFetcher.h"
#include "adapters/LocalLogoStore.h"
#include "core/FlightDataFetcher.h"
#include "adapters/NeoMatrixDisplay.h"

static OpenSkyFetcher    g_openSky;
static AeroAPIFetcher    g_aeroApi;
static LocalLogoStore    g_logoStore;
static FlightDataFetcher *g_fetcher = nullptr;
static NeoMatrixDisplay  g_display;

static unsigned long g_lastFetchMs = 0;
static unsigned long g_lastDisplayRotationMs = 0;

// Global storage for rotation
static std::vector<FlightInfo> g_currentFlights;
static int g_displayIndex = 0;

void setup() {
    Serial.begin(115200);
    delay(200);

    g_display.initialize();
    g_display.displayMessage(String("FlightWall"));
    g_logoStore.initialize();

    if (strlen(WiFiConfiguration::WIFI_SSID) > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(WiFiConfiguration::WIFI_SSID, WiFiConfiguration::WIFI_PASSWORD);
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 50) {
            delay(200);
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            g_display.displayMessage(String("WiFi OK"));
            delay(1000);
            g_display.showLoading();
        }
    }

    g_fetcher = new FlightDataFetcher(&g_openSky, &g_aeroApi, &g_logoStore);
}

void loop() {
    unsigned long now = millis();
    const unsigned long fetchIntervalMs = TimingConfiguration::FETCH_INTERVAL_SECONDS * 1000UL;
    const unsigned long displayIntervalMs = TimingConfiguration::DISPLAY_CYCLE_SECONDS * 1000UL;

    // --- TASK 1: FETCH DATA (Every 90 Seconds) ---
    if (now - g_lastFetchMs >= fetchIntervalMs || g_lastFetchMs == 0) {
        g_lastFetchMs = now;

        std::vector<StateVector> states;
        std::vector<FlightInfo> newFlights;
        
        // This makes the AeroAPI call
        size_t enriched = g_fetcher->fetchFlights(states, newFlights);
        
        if (enriched > 0) {
            g_currentFlights = newFlights;
            // Only reset index if we aren't mid-rotation
            if (g_displayIndex >= g_currentFlights.size()) g_displayIndex = 0;
        }
        
        Serial.printf("API Fetch: %d flights enriched.\n", (int)enriched);
    }

    // --- TASK 2: ROTATE DISPLAY (Every 25 Seconds) ---
    if (now - g_lastDisplayRotationMs >= displayIntervalMs || g_lastDisplayRotationMs == 0) {
        g_lastDisplayRotationMs = now;

        if (!g_currentFlights.empty()) {
            // Safety check for index
            if (g_displayIndex >= g_currentFlights.size()) g_displayIndex = 0;

            // Prepare a temporary vector containing ONLY the flight to show
            std::vector<FlightInfo> singleFlight;
            singleFlight.push_back(g_currentFlights[g_displayIndex]);

            Serial.printf("Displaying Flight %d/%d: %s\n", 
                          g_displayIndex + 1, 
                          (int)g_currentFlights.size(), 
                          g_currentFlights[g_displayIndex].ident.c_str());

            g_display.displayFlights(singleFlight);

            // Increment for next cycle
            g_displayIndex = (g_displayIndex + 1) % g_currentFlights.size();
        } else {
            // No flights in range
            g_display.displayMessage("Scanning...");
        }
    }

    delay(50); // General stability
}
