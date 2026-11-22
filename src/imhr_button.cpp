#include "imhr_button.h"
#include "imhr_button_logic.h"

namespace imhr
{

    static uint8_t buttonPin;
    static Debouncer debouncer;

    void buttonInit(uint8_t pin)
    {
        buttonPin = pin;
        pinMode(buttonPin, INPUT_PULLUP);
    }

    bool buttonWasPressed()
    {
        bool rawPressed = digitalRead(buttonPin) == LOW;
        return debouncer.update(rawPressed, millis());
    }

}
