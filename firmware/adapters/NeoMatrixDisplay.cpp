#pragma once

#include <stdint.h>
#include <vector>
#include <LittleFS.h>
#include "interfaces/BaseDisplay.h"

// Forward declarations to keep the header lean
class FastLED_NeoMatrix;
struct CRGB;

class NeoMatrixDisplay : public BaseDisplay
{
public:
    NeoMatrixDisplay();
    ~NeoMatrixDisplay() override;

    bool initialize() override;
    void clear() override;
    
    // This is called by your main loop to start the drawing process
    void displayFlights(const std::vector<FlightInfo> &flights) override;
    
    // For "Scanning..." or "WiFi OK" messages
    void displayMessage(const String &message);

private:
    FastLED_NeoMatrix *_matrix = nullptr;
    
    // --- Core Drawing Functions ---
    // Handles the 16x16 .bmp files from your /data folder
    void drawLogoFromFS(int16_t x, int16_t y, const String &path);
    
    // The main layout engine for your 3x2 panel grid
    void displaySingleFlightCard(const FlightInfo &f);
    
    // Helper to keep the .cpp code clean and readable
    void drawText(int16_t x, int16_t y, const String &text, uint16_t color);

    // --- Updated Layout Dimensions for 3 Wide x 2 Tall ---
    // This matches your physical 48x32 LED matrix
    const uint16_t M_WIDTH  = 48; 
    const uint16_t M_HEIGHT = 32;
};
