#pragma once

#include <Arduino.h>

namespace imhr
{

    void buttonInit(uint8_t pin);
    bool buttonWasPressed();

}
