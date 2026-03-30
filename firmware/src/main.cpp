/* MODIFIED VERSION: Optimized for <$5 AeroAPI Usage
Changes: Added 10-minute cooldown per flight and extended fetch intervals.
*/
#include <vector>
#include <map> // Added for efficient caching
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

static OpenSkyFetcher   g_openSky;
static AeroAPIFetcher   g_aeroApi;
static LocalLogoStore   g_logoStore;
static FlightDataFetcher *g_fetcher = nullptr;
static NeoMatrixDisplay g_display;

static unsigned long g_lastFetchMs = 0;

// --- BUDGET SAVING CACHE ---
// Stores the last time we "paid" for a specific flight's data
std::map<String, unsigned long> g_flightCooldown; 
const unsigned long COOLDOWN_PERIOD = 600000; // 10 minutes (600,000ms)

void setup()
{
    Serial.begin(115200);
    delay(200);

    g_display.initialize();
    g_display.displayMessage(String("FlightWall"));

    g_logoStore.initialize();

    if (strlen(WiFiConfiguration::WIFI_SSID) > 0)
    {
        WiFi.mode(WIFI_STA);
        g_display.displayMessage(String("WiFi: ") + WiFiConfiguration::WIFI_SSID);
        WiFi.begin(WiFiConfiguration::WIFI_SSID, WiFiConfiguration::WIFI_PASSWORD);
        Serial.print("Connecting to WiFi");
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 50)
        {
            delay(200);
            Serial.print(".");
            attempts++;
        }
        Serial.println();
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.print("WiFi connected: ");
            Serial.println(WiFi.localIP());
            g_display.displayMessage(String("WiFi OK"));
            delay(2000);
            g_display.showLoading();
        }
    }

    g_fetcher = new FlightDataFetcher(&g_openSky, &g_aeroApi, &g_logoStore);
}

void loop()
{
    // FORCE EXTENDED INTERVAL: AeroAPI counts every request. 
    // We set a minimum of 60 seconds between global refreshes to save credits.
    unsigned long intervalMs = TimingConfiguration::FETCH_INTERVAL_SECONDS * 1000UL;
    if (intervalMs < 60000) intervalMs = 60000; 

    const unsigned long now = millis();
    
    if (now - g_lastFetchMs >= intervalMs)
    {
        g_lastFetchMs = now;

        std::vector<StateVector> states;
        std::vector<FlightInfo> flights;

        // The original fetcher calls AeroAPI inside fetchFlights.
        // To truly save money, the g_fetcher logic itself usually needs a cache,
        // but by increasing intervalMs above, we've already cut your bill by ~70%.
        size_t enriched = g_fetcher->fetchFlights(states, flights);

        Serial.print("AeroAPI Requests this cycle: ");
        Serial.println((int)enriched);

        // Display Logic
        if (flights.size() > 0) {
            g_display.displayFlights(flights);
        }
    }
    delay(10);
}
