#include "imhr_display_logic.h"
#include <cmath>
#include <cstdio>

namespace imhr
{

    static const char *weekdays[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};

    void determineMinuteDisplay(char *minuteDisplay, size_t size, int minutes)
    {
        if (minutes < 0)
            snprintf(minuteDisplay, size, "--");
        else if (minutes == 0)
            snprintf(minuteDisplay, size, "sofort");
        else
            snprintf(minuteDisplay, size, "%d min", minutes);
    }

    void formatTemperature(char *buffer, size_t size, float celsius)
    {
        snprintf(buffer, size, "%ld°", lroundf(celsius));
    }

    void formatDate(char *buffer, size_t size, const tm &time)
    {
        snprintf(buffer, size, "%s %02d.%02d.", weekdays[time.tm_wday], time.tm_mday, time.tm_mon + 1);
    }

}
