#pragma once

#include <Arduino.h>

namespace HardwareConfiguration
{
    // Data pin on your ESP32
    static const uint8_t DISPLAY_PIN = 25; 

    // Physical panel size — your WS2812B 16x16 panel
    static const uint16_t DISPLAY_TILE_PIXEL_W = 16;
    static const uint16_t DISPLAY_TILE_PIXEL_H = 16;

    // Arrangement: Set to 1x1 if using just one panel
    static const uint8_t DISPLAY_TILES_X = 2;
    static const uint8_t DISPLAY_TILES_Y = 3;

    // Total matrix dimensions: 16x16
    static const uint16_t DISPLAY_MATRIX_WIDTH  = DISPLAY_TILE_PIXEL_W * DISPLAY_TILES_X;
    static const uint16_t DISPLAY_MATRIX_HEIGHT = DISPLAY_TILE_PIXEL_H * DISPLAY_TILES_Y;
    
    // BRIGHTNESS NOTE: 256 LEDs draw a lot of power. 
    // Ensure your 5V power supply is at least 2A-4A.
    static const uint8_t MATRIX_BRIGHTNESS = 30; 
}
