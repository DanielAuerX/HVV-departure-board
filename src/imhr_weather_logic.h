#pragma once

#include <ArduinoJson.h>

namespace imhr
{

    enum class WeatherCondition
    {
        Clear,
        PartlyCloudy,
        Cloudy,
        Fog,
        Drizzle,
        Rain,
        Snow,
        Thunderstorm,
        Unknown
    };

    struct Weather
    {
        bool valid;
        float temperature;
        float temperatureMin;
        float temperatureMax;
        int precipitationProbability;
        WeatherCondition condition;
    };

    WeatherCondition conditionFromWmoCode(int code);
    const char *conditionLabel(WeatherCondition condition);
    Weather parseWeather(JsonDocument &doc);

}
