#include "imhr_ventilation_logic.h"

namespace imhr
{

    static const int humidityOnThreshold = 60;
    static const int humidityOffThreshold = 55;

    bool ventilationNeeded(int humidity, bool currentlyNeeded)
    {
        if (currentlyNeeded)
            return humidity >= humidityOffThreshold;
        return humidity >= humidityOnThreshold;
    }

}
