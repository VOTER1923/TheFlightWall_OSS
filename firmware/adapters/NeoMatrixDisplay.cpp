#pragma once

#include <stdint.h>
#include <vector>
#include <LittleFS.h>
#include "interfaces/BaseDisplay.h"

class FastLED_NeoMatrix;
struct CRGB;

class NeoMatrixDisplay : public BaseDisplay
{
public:
    NeoMatrixDisplay();
    ~NeoMatrixDisplay() override;

    bool initialize() override;
    void clear() override;
    void displayFlights(const std::vector<FlightInfo> &flights) override;
    void displayMessage(const String &message);

private:
    FastLED_NeoMatrix *_matrix = nullptr;
    
    // Core Drawing Functions
    void drawLogoFromFS(int16_t x, int16_t y, const String &path);
    void displaySingleFlightCard(const FlightInfo &f);
    void drawText(int16_t x, int16_t y, const String &text, uint16_t color);

    // Layout dimensions for 2x3 (32x48)
    const uint16_t M_WIDTH = 32;
    const uint16_t M_HEIGHT = 48;
};
