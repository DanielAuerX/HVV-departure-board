#pragma once

#include <cstdint>

namespace imhr
{

    constexpr int dhtFrameBits = 40;

    struct IndoorClimate
    {
        bool valid;
        float temperature;
        int humidity;
    };

    IndoorClimate decodeDhtPulses(const uint16_t highPulsesUs[dhtFrameBits]);

}
