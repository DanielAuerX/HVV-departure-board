#include "imhr_dht.h"

namespace imhr
{

    static const uint32_t startSignalMs = 20;
    static const uint32_t levelTimeoutUs = 100;

    static uint8_t dhtPin;
    static portMUX_TYPE dhtMux = portMUX_INITIALIZER_UNLOCKED;

    void dhtInit(uint8_t pin)
    {
        dhtPin = pin;
        pinMode(dhtPin, OUTPUT_OPEN_DRAIN | PULLUP);
        digitalWrite(dhtPin, HIGH);
    }

    static int32_t measureLevel(int level)
    {
        uint32_t start = micros();
        while (digitalRead(dhtPin) == level)
        {
            if (micros() - start > levelTimeoutUs)
                return -1;
        }
        return micros() - start;
    }

    static void sendStartSignal()
    {
        digitalWrite(dhtPin, LOW);
        delay(startSignalMs);
    }

    static bool awaitResponse()
    {
        return measureLevel(HIGH) >= 0 && measureLevel(LOW) >= 0 && measureLevel(HIGH) >= 0;
    }

    static bool readPulses(uint16_t highPulsesUs[dhtFrameBits])
    {
        if (!awaitResponse())
            return false;

        for (int bit = 0; bit < dhtFrameBits; bit++)
        {
            if (measureLevel(LOW) < 0)
                return false;

            int32_t high = measureLevel(HIGH);
            if (high < 0)
                return false;

            highPulsesUs[bit] = high;
        }
        return true;
    }

    IndoorClimate readIndoorClimate()
    {
        uint16_t highPulsesUs[dhtFrameBits];

        sendStartSignal();
        portENTER_CRITICAL(&dhtMux);
        digitalWrite(dhtPin, HIGH);
        bool complete = readPulses(highPulsesUs);
        portEXIT_CRITICAL(&dhtMux);

        if (!complete)
        {
            Serial.println("DHT11 error: no response");
            return {false, 0, 0};
        }

        IndoorClimate climate = decodeDhtPulses(highPulsesUs);
        if (!climate.valid)
        {
            Serial.println("DHT11 error: checksum mismatch");
            return climate;
        }

        Serial.printf("indoor: %.1f C, %d%% humidity\n", climate.temperature, climate.humidity);
        return climate;
    }

}
