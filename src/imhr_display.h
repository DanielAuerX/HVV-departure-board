#pragma once
#include <U8g2lib.h>
#include "imhr_weather_logic.h"
#include "imhr_dht_logic.h"

namespace imhr
{
    void displayInit();
    void displayWake();
    void displaySleep();
    void displayMessage(const char *line1, const char *line2 = nullptr);
    void displayDeparture(const char *busLine, int busMins,
                          const char *trainPlatform, int trainMins);
    void displayWeather(const Weather &weather, const IndoorClimate &indoor, const tm *time);
    void tickScroll();
    void setAnnouncementMessage(const char *message);
}