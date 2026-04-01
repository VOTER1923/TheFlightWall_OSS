#include "adapters/LocalLogoStore.h"
#include <LittleFS.h>

bool LocalLogoStore::initialize() {
    if (!LittleFS.begin(false)) {
        Serial.println("LocalLogoStore: LittleFS mount failed.");
        _mounted = false;
        return false;
    }
    _mounted = true;
    Serial.println("LocalLogoStore: LittleFS mounted OK.");
    return true;
}

// This matches the logic we used in the Display class
bool LocalLogoStore::getAirlineLogo(const String &airlineIcao, std::vector<uint16_t> &outPixels) {
    outPixels.clear();
    if (!_mounted) return false;

    String icao = airlineIcao;
    icao.toUpperCase();
    
    // We are looking for .bmp files in the root /data folder
    String path = "/" + icao + ".bmp";

    if (!LittleFS.exists(path)) return false;

    File f = LittleFS.open(path, "r");
    if (!f) return false;

    // Skip the 54-byte BMP header
    f.seek(54);

    // Read 16x16 pixels (3 bytes each for 24-bit BMP)
    for (int i = 0; i < 256; i++) {
        uint8_t b = f.read();
        uint8_t g = f.read();
        uint8_t r = f.read();
        
        // Convert 24-bit RGB to 16-bit RGB565 (which the matrix uses)
        uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        outPixels.push_back(rgb565);
    }

    f.close();
    return true;
}
