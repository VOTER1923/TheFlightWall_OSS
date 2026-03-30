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
    // 720s = 12 minutes. This is your "Money Saver" timer.
    const unsigned long fetchIntervalMs = 720 * 1000UL; 
    const unsigned long displayIntervalMs = 25 * 1000UL;

    static std::vector<FlightInfo> g_currentFlights;
    static int g_displayIndex = 0;

    // --- 1. THE FETCH (The $0.02 moment) ---
    if (now - g_lastFetchMs >= fetchIntervalMs || g_lastFetchMs == 0) {
        g_lastFetchMs = now;
        
        std::vector<StateVector> states;
        std::vector<FlightInfo> newFlights;
        
        // Fetch everything nearby
        g_fetcher->fetchFlights(states, newFlights);
        
        // LIMIT TO 8 BEST FLIGHTS (Closest to you)
        if (newFlights.size() > 8) {
            newFlights.resize(8);
        }
        
        g_currentFlights = newFlights;
        g_displayIndex = 0; // Start at the beginning of the new batch
        Serial.println("New 12-minute batch loaded. Budget safe.");
    }

    // --- 2. THE ROTATION (The Free Part) ---
    if (now - g_lastDisplayRotationMs >= displayIntervalMs) {
        g_lastDisplayRotationMs = now;

        if (!g_currentFlights.empty()) {
            // Pick the next flight in our cache of 8
            std::vector<FlightInfo> singleFlight;
            singleFlight.push_back(g_currentFlights[g_displayIndex]);

            g_display.displayFlights(singleFlight);

            // Move to next (1 through 8) and loop back
            g_displayIndex = (g_displayIndex + 1) % g_currentFlights.size();
        } else {
            g_display.displayMessage("Sky Clear");
        }
    }
    delay(50);
}
