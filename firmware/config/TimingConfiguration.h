#pragma once

#include <Arduino.h>

namespace TimingConfiguration
{
    // Increased to 60s to stay under $5 AeroAPI limit
    static const uint32_t FETCH_INTERVAL_SECONDS = 90; 

    // As requested: Each flight stays on screen for 25 seconds
    static const uint32_t DISPLAY_CYCLE_SECONDS = 25; 
}
