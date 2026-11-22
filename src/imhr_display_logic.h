#pragma once

#include <cstddef>
#include <ctime>

namespace imhr
{

    void determineMinuteDisplay(char *minuteDisplay, size_t size, int minutes);
    void formatTemperature(char *buffer, size_t size, float celsius);
    void formatDate(char *buffer, size_t size, const tm &time);

}
