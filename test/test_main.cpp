#include <gtest/gtest.h>
#include <ArduinoJson.h>
#include "imhr_departure_logic.h"
#include "imhr_display_logic.h"
#include "imhr_weather_logic.h"
#include "imhr_button_logic.h"
#include "imhr_dht_logic.h"

using namespace imhr;

TEST(GetDelayInMinutes, ReturnsZeroWhenDelayIsMissing)
{
    JsonDocument doc;
    JsonObject departure = doc.to<JsonObject>();

    EXPECT_EQ(getDelayInMinutes(departure), 0);
}

TEST(GetDelayInMinutes, ConvertsSecondsToMinutes)
{
    JsonDocument doc;
    doc["delay"] = 180;
    JsonObject departure = doc.as<JsonObject>();

    EXPECT_EQ(getDelayInMinutes(departure), 3);
}

TEST(GetDelayInMinutes, RoundsDownPartialMinutes)
{
    JsonDocument doc;
    doc["delay"] = 119;
    JsonObject departure = doc.as<JsonObject>();

    EXPECT_EQ(getDelayInMinutes(departure), 1);
}

TEST(FormatPlatformLabel, PrefersRealtimePlatformAndStripsPrefix)
{
    JsonDocument doc;
    doc["line"]["name"] = "S3";
    doc["realtimePlatform"] = "Gleis 12";
    doc["platform"] = "Gleis 3";
    JsonObject departure = doc.as<JsonObject>();

    EXPECT_EQ(formatPlatformLabel(departure), "S3-12");
}

TEST(FormatPlatformLabel, FallsBackToPlatformWhenRealtimeMissing)
{
    JsonDocument doc;
    doc["line"]["name"] = "U1";
    doc["platform"] = "Gleis 4";
    JsonObject departure = doc.as<JsonObject>();

    EXPECT_EQ(formatPlatformLabel(departure), "U1-4");
}

TEST(FormatPlatformLabel, ReturnsJustLineNameWhenNoPlatform)
{
    JsonDocument doc;
    doc["line"]["name"] = "19";
    JsonObject departure = doc.as<JsonObject>();

    EXPECT_EQ(formatPlatformLabel(departure), "19");
}

TEST(DetermineMinuteDisplay, ShowsDashesForNegativeMinutes)
{
    char buf[12];
    determineMinuteDisplay(buf, sizeof(buf), -1);
    EXPECT_STREQ(buf, "--");
}

TEST(DetermineMinuteDisplay, ShowsSofortForZeroMinutes)
{
    char buf[12];
    determineMinuteDisplay(buf, sizeof(buf), 0);
    EXPECT_STREQ(buf, "sofort");
}

TEST(DetermineMinuteDisplay, ShowsMinutesCount)
{
    char buf[12];
    determineMinuteDisplay(buf, sizeof(buf), 7);
    EXPECT_STREQ(buf, "7 min");
}

TEST(ConditionFromWmoCode, MapsCodeGroups)
{
    EXPECT_EQ(conditionFromWmoCode(0), WeatherCondition::Clear);
    EXPECT_EQ(conditionFromWmoCode(2), WeatherCondition::PartlyCloudy);
    EXPECT_EQ(conditionFromWmoCode(3), WeatherCondition::Cloudy);
    EXPECT_EQ(conditionFromWmoCode(45), WeatherCondition::Fog);
    EXPECT_EQ(conditionFromWmoCode(53), WeatherCondition::Drizzle);
    EXPECT_EQ(conditionFromWmoCode(63), WeatherCondition::Rain);
    EXPECT_EQ(conditionFromWmoCode(81), WeatherCondition::Rain);
    EXPECT_EQ(conditionFromWmoCode(75), WeatherCondition::Snow);
    EXPECT_EQ(conditionFromWmoCode(86), WeatherCondition::Snow);
    EXPECT_EQ(conditionFromWmoCode(95), WeatherCondition::Thunderstorm);
}

TEST(ConditionFromWmoCode, ReturnsUnknownForUnmappedCode)
{
    EXPECT_EQ(conditionFromWmoCode(42), WeatherCondition::Unknown);
}

TEST(ConditionLabel, ReturnsGermanLabel)
{
    EXPECT_STREQ(conditionLabel(WeatherCondition::Rain), "Regen");
    EXPECT_STREQ(conditionLabel(WeatherCondition::Unknown), "?");
}

TEST(ParseWeather, ReadsCurrentAndDailyValues)
{
    JsonDocument doc;
    deserializeJson(doc, R"({
        "current": {"temperature_2m": 3.9, "weather_code": 3},
        "daily": {"temperature_2m_max": [4.6], "temperature_2m_min": [-0.5], "precipitation_probability_max": [8]}
    })");

    Weather weather = parseWeather(doc);

    EXPECT_TRUE(weather.valid);
    EXPECT_FLOAT_EQ(weather.temperature, 3.9f);
    EXPECT_FLOAT_EQ(weather.temperatureMin, -0.5f);
    EXPECT_FLOAT_EQ(weather.temperatureMax, 4.6f);
    EXPECT_EQ(weather.precipitationProbability, 8);
    EXPECT_EQ(weather.condition, WeatherCondition::Cloudy);
}

TEST(ParseWeather, IsInvalidWhenCurrentIsMissing)
{
    JsonDocument doc;
    deserializeJson(doc, R"({"daily": {"temperature_2m_max": [4.6]}})");

    EXPECT_FALSE(parseWeather(doc).valid);
}

TEST(FormatTemperature, RoundsToWholeDegrees)
{
    char buf[8];
    formatTemperature(buf, sizeof(buf), 3.6f);
    EXPECT_STREQ(buf, "4°");
}

TEST(FormatTemperature, ShowsNegativeValues)
{
    char buf[8];
    formatTemperature(buf, sizeof(buf), -2.4f);
    EXPECT_STREQ(buf, "-2°");
}

TEST(FormatTemperature, ShowsZeroInsteadOfNegativeZero)
{
    char buf[8];
    formatTemperature(buf, sizeof(buf), -0.4f);
    EXPECT_STREQ(buf, "0°");
}

TEST(FormatDate, ShowsGermanWeekdayAndDate)
{
    tm time = {};
    time.tm_wday = 6;
    time.tm_mday = 22;
    time.tm_mon = 10;
    char buf[12];
    formatDate(buf, sizeof(buf), time);
    EXPECT_STREQ(buf, "Sa 22.11.");
}

TEST(Debouncer, ReportsStablePressOnce)
{
    Debouncer debouncer;
    EXPECT_FALSE(debouncer.update(true, 0));
    EXPECT_FALSE(debouncer.update(true, 10));
    EXPECT_TRUE(debouncer.update(true, 40));
    EXPECT_FALSE(debouncer.update(true, 100));
}

TEST(Debouncer, IgnoresBouncing)
{
    Debouncer debouncer;
    EXPECT_FALSE(debouncer.update(true, 0));
    EXPECT_FALSE(debouncer.update(false, 5));
    EXPECT_FALSE(debouncer.update(true, 10));
    EXPECT_FALSE(debouncer.update(true, 35));
    EXPECT_TRUE(debouncer.update(true, 41));
}

TEST(Debouncer, DoesNotReportRelease)
{
    Debouncer debouncer;
    debouncer.update(true, 0);
    EXPECT_TRUE(debouncer.update(true, 40));
    EXPECT_FALSE(debouncer.update(false, 50));
    EXPECT_FALSE(debouncer.update(false, 100));
}

TEST(Debouncer, ReportsSecondPressAfterRelease)
{
    Debouncer debouncer;
    debouncer.update(true, 0);
    debouncer.update(true, 40);
    debouncer.update(false, 50);
    debouncer.update(false, 100);
    EXPECT_FALSE(debouncer.update(true, 150));
    EXPECT_TRUE(debouncer.update(true, 200));
}

static void bytesToPulses(const uint8_t data[5], uint16_t pulses[dhtFrameBits])
{
    const uint16_t zeroPulseUs = 27;
    const uint16_t onePulseUs = 70;
    for (int bit = 0; bit < dhtFrameBits; bit++)
    {
        bool isOne = data[bit / 8] & (0x80 >> (bit % 8));
        pulses[bit] = isOne ? onePulseUs : zeroPulseUs;
    }
}

TEST(DecodeDhtPulses, DecodesTemperatureAndHumidity)
{
    const uint8_t data[5] = {45, 0, 21, 3, 69};
    uint16_t pulses[dhtFrameBits];
    bytesToPulses(data, pulses);

    IndoorClimate climate = decodeDhtPulses(pulses);

    EXPECT_TRUE(climate.valid);
    EXPECT_FLOAT_EQ(climate.temperature, 21.3f);
    EXPECT_EQ(climate.humidity, 45);
}

TEST(DecodeDhtPulses, DecodesNegativeTemperature)
{
    const uint8_t data[5] = {80, 0, 2, 0x85, 215};
    uint16_t pulses[dhtFrameBits];
    bytesToPulses(data, pulses);

    EXPECT_FLOAT_EQ(decodeDhtPulses(pulses).temperature, -2.5f);
}

TEST(DecodeDhtPulses, IsInvalidOnChecksumMismatch)
{
    const uint8_t data[5] = {45, 0, 21, 3, 70};
    uint16_t pulses[dhtFrameBits];
    bytesToPulses(data, pulses);

    EXPECT_FALSE(decodeDhtPulses(pulses).valid);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
