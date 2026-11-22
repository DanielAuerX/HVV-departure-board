#include "imhr_led.h"

namespace imhr
{

    static uint8_t ledPin;

    void ledInit(uint8_t pin)
    {
        ledPin = pin;
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, LOW);
    }

    void ledSet(bool on)
    {
        digitalWrite(ledPin, on ? HIGH : LOW);
    }

}
