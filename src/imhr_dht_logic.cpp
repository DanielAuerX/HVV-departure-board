#include "imhr_dht_logic.h"

namespace imhr
{

    static const int dhtFrameBytes = dhtFrameBits / 8;
    static const uint16_t oneBitThresholdUs = 50;
    static const uint8_t negativeFlag = 0x80;
    static const uint8_t decimalMask = 0x0F;

    static void pulsesToBytes(const uint16_t highPulsesUs[dhtFrameBits], uint8_t data[dhtFrameBytes])
    {
        for (int bit = 0; bit < dhtFrameBits; bit++)
        {
            data[bit / 8] <<= 1;
            if (highPulsesUs[bit] > oneBitThresholdUs)
                data[bit / 8] |= 1;
        }
    }

    static bool checksumMatches(const uint8_t data[dhtFrameBytes])
    {
        uint8_t sum = data[0] + data[1] + data[2] + data[3];
        return sum == data[4];
    }

    IndoorClimate decodeDhtPulses(const uint16_t highPulsesUs[dhtFrameBits])
    {
        uint8_t data[dhtFrameBytes] = {};
        pulsesToBytes(highPulsesUs, data);

        if (!checksumMatches(data))
            return {false, 0, 0};

        float temperature = data[2] + (data[3] & decimalMask) * 0.1f;
        if (data[3] & negativeFlag)
            temperature = -temperature;

        return {true, temperature, data[0]};
    }

}
