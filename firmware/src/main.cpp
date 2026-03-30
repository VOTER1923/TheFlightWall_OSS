/*
Modified FlightWall Firmware - Whittlesea Budget Edition
- Optimized for <$5 AeroAPI monthly usage (16 hours/day operation).
- 12-minute fetch interval to stay within free credits.
- 25-second per-flight display rotation.
- Capped at 8 flights per refresh to maximize relevance and minimize costs.
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
    
    // Budget Timings: 720s (12m) Fetch / 25s Display
    const unsigned long fetchIntervalMs = 720 * 1000UL; 
    const unsigned long displayIntervalMs = 25 * 1000UL;

    // --- TASK 1: FETCH DATA (The "Paid" Part - Every 12 Minutes) ---
    if (now - g_lastFetchMs >= fetchIntervalMs || g_lastFetchMs == 0) {
        g_lastFetchMs = now;

        std::vector<StateVector> states;
        std::vector<FlightInfo> newFlights;
        
        // Fetch and enrich flights via AeroAPI
        size_t enriched = g_fetcher->fetchFlights(states, newFlights);
        
        // BUDGET CAP: Limit to 8 flights to ensure the rotation stays fresh 
        // and doesn't exceed API result set limits.
        if (newFlights.size() > 8) {
            newFlights.resize(8);
            enriched = 8;
        }
        
        if (enriched > 0) {
            g_currentFlights = newFlights;
            g_displayIndex = 0; // Restart rotation with fresh data
        } else {
            g_currentFlights.clear();
        }
        
        Serial.printf("AeroAPI Refresh: %d flights cached. Next refresh in 12m.\n", (int)enriched);
    }

    // --- TASK 2: ROTATE DISPLAY (The "Free" Part - Every 25 Seconds) ---
    if (now - g_lastDisplayRotationMs >= displayIntervalMs || g_lastDisplayRotationMs == 0) {
        g_lastDisplayRotationMs = now;

        if (!g_currentFlights.empty()) {
            // Index safety check
            if (g_displayIndex >= g_currentFlights.size()) g_displayIndex = 0;

            // Extract single flight for the matrix display
            std::vector<FlightInfo> singleFlight;
            singleFlight.push_back(g_currentFlights[g_displayIndex]);

            Serial.printf("Showing [%d/%d]: %s\n", 
                          g_displayIndex + 1, 
                          (int)g_currentFlights.size(), 
                          g_currentFlights[g_displayIndex].ident.c_str());

            g_display.displayFlights(singleFlight);

            // Move to next flight in cache
            g_displayIndex = (g_displayIndex + 1) % g_currentFlights.size();
        } else {
            g_display.displayMessage("Scanning...");
        }
    }

    delay(50); 
}
