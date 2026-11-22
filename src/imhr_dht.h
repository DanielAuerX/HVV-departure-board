#pragma once

#include <Arduino.h>
#include "imhr_dht_logic.h"

namespace imhr
{

    void dhtInit(uint8_t pin);
    IndoorClimate readIndoorClimate();

}
