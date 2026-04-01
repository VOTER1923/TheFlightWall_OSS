#pragma once

#include <Arduino.h>

namespace HardwareConfiguration
{
    // Data pin on your ESP32
    static const uint8_t DISPLAY_PIN = 25; 

    // Physical panel size — your WS2812B 16x16 panel
    static const uint16_t DISPLAY_TILE_PIXEL_W = 16;
    static const uint16_t DISPLAY_TILE_PIXEL_H = 16;

    // --- UPDATED FOR YOUR 3x2 LANDSCAPE SETUP ---
    static const uint8_t DISPLAY_TILES_X = 3; // 3 panels wide (48 pixels)
    static const uint8_t DISPLAY_TILES_Y = 2; // 2 panels tall (32 pixels)

    // Total matrix dimensions: 48x32
    static const uint16_t DISPLAY_MATRIX_WIDTH  = DISPLAY_TILE_PIXEL_W * DISPLAY_TILES_X; // 48
    static const uint16_t DISPLAY_MATRIX_HEIGHT = DISPLAY_TILE_PIXEL_H * DISPLAY_TILES_Y; // 32
    
    // BRIGHTNESS NOTE: 30 is a safe start for Whittlesea daytime.
    // If you are powered by a beefy 5V 10A supply, you can go higher later.
    static const uint8_t MATRIX_BRIGHTNESS = 30; 
}
