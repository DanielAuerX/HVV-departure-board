#include "imhr_weather.h"
#include "imhr_endpoints.h"
#include "imhr_json_keys.h"
#include "imhr_http_client.h"
#include "imhr_secrets.h"

namespace imhr
{

    static String buildForecastUrl()
    {
        return String(BASE_URL_OPEN_METEO) + ENDPOINT_FORECAST +
               "?latitude=" + WEATHER_LATITUDE +
               "&longitude=" + WEATHER_LONGITUDE +
               "&current=" + JSON_KEY_TEMPERATURE + "," + JSON_KEY_WEATHER_CODE +
               "&daily=" + JSON_KEY_TEMPERATURE_MIN + "," + JSON_KEY_TEMPERATURE_MAX + "," + JSON_KEY_PRECIPITATION_PROBABILITY_MAX +
               "&forecast_days=1&timezone=Europe%2FBerlin";
    }

    static void logWeather(const Weather &weather)
    {
        Serial.printf("weather: %.1f C, %s (min %.1f, max %.1f, rain %d%%)\n",
                      weather.temperature, conditionLabel(weather.condition),
                      weather.temperatureMin, weather.temperatureMax, weather.precipitationProbability);
    }

    Weather fetchWeather()
    {
        HttpResponse response = sendGetRequest(buildForecastUrl());
        if (!response.success())
        {
            Serial.printf("Weather HTTP error: %d\n", response.code);
            return {false, 0, 0, 0, 0, WeatherCondition::Unknown};
        }

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response.body);
        if (error)
        {
            Serial.printf("Weather JSON error: %s\n", error.c_str());
            return {false, 0, 0, 0, 0, WeatherCondition::Unknown};
        }

        Weather weather = parseWeather(doc);
        logWeather(weather);
        return weather;
    }

}
