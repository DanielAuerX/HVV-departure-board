#include "imhr_weather_logic.h"
#include "imhr_json_keys.h"

namespace imhr
{

    WeatherCondition conditionFromWmoCode(int code)
    {
        if (code == 0)
            return WeatherCondition::Clear;
        if (code == 1 || code == 2)
            return WeatherCondition::PartlyCloudy;
        if (code == 3)
            return WeatherCondition::Cloudy;
        if (code == 45 || code == 48)
            return WeatherCondition::Fog;
        if (code >= 51 && code <= 57)
            return WeatherCondition::Drizzle;
        if ((code >= 61 && code <= 67) || (code >= 80 && code <= 82))
            return WeatherCondition::Rain;
        if ((code >= 71 && code <= 77) || code == 85 || code == 86)
            return WeatherCondition::Snow;
        if (code >= 95 && code <= 99)
            return WeatherCondition::Thunderstorm;
        return WeatherCondition::Unknown;
    }

    const char *conditionLabel(WeatherCondition condition)
    {
        switch (condition)
        {
        case WeatherCondition::Clear:
            return "klar";
        case WeatherCondition::PartlyCloudy:
            return "heiter";
        case WeatherCondition::Cloudy:
            return "bewoelkt";
        case WeatherCondition::Fog:
            return "Nebel";
        case WeatherCondition::Drizzle:
            return "Niesel";
        case WeatherCondition::Rain:
            return "Regen";
        case WeatherCondition::Snow:
            return "Schnee";
        case WeatherCondition::Thunderstorm:
            return "Gewitter";
        default:
            return "?";
        }
    }

    Weather parseWeather(JsonDocument &doc)
    {
        JsonObject current = doc[JSON_KEY_CURRENT];
        JsonObject daily = doc[JSON_KEY_DAILY];

        if (current[JSON_KEY_TEMPERATURE].isNull() || current[JSON_KEY_WEATHER_CODE].isNull())
            return {false, 0, 0, 0, 0, WeatherCondition::Unknown};

        return {
            true,
            current[JSON_KEY_TEMPERATURE].as<float>(),
            daily[JSON_KEY_TEMPERATURE_MIN][0].as<float>(),
            daily[JSON_KEY_TEMPERATURE_MAX][0].as<float>(),
            daily[JSON_KEY_PRECIPITATION_PROBABILITY_MAX][0].as<int>(),
            conditionFromWmoCode(current[JSON_KEY_WEATHER_CODE].as<int>())};
    }

}
