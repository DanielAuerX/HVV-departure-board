#include <Wire.h>
#include "imhr_wifi.h"
#include "imhr_display.h"
#include "imhr_announcement.h"
#include "imhr_departure.h"
#include "imhr_weather.h"
#include "imhr_button.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

const uint32_t wifiRetryDelay = 600000;      // 10 minutes
const uint32_t sleepRetryDelay = 600000;     // 10 minutes
const uint32_t readableMessageDelay = 2000;  // 2 seconds
const uint32_t shortPollingInterval = 30000; // 30 seconds
const uint32_t announcementPollingInterval = shortPollingInterval * 20; // 10 minutes
const uint32_t weatherPollingInterval = shortPollingInterval * 30;
const char *timeZone = "CET-1CEST,M3.5.0,M10.5.0/3";
const char *ntpServer = "pool.ntp.org";
const int sleepStartHour = 0;
const int sleepStartMin = 30;
const int sleepEndHour = 5;
const int sleepEndMin = 0;
const uint8_t viewButtonPin = 27;
int counter = 0;

enum class View
{
  Departure,
  Weather
};

struct Updates
{
  bool departure;
  bool weather;
};

static SemaphoreHandle_t dataMutex;

static imhr::BusDeparture sharedBus = {"bus", -1};
static imhr::TrainDeparture sharedTrain = {"train", -1};
static imhr::Announcement sharedAnnouncement = {false, ""};
static imhr::Weather sharedWeather = {false, 0, 0, 0, 0, imhr::WeatherCondition::Unknown};
static bool departureDirty = false;
static bool announcementDirty = false;
static bool weatherDirty = false;

static View currentView = View::Departure;
static imhr::BusDeparture shownBus = sharedBus;
static imhr::TrainDeparture shownTrain = sharedTrain;
static imhr::Weather shownWeather = sharedWeather;
static int shownMinute = -1;

static bool isNightTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
    return false;

  int nowMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  int startMinutes = sleepStartHour * 60 + sleepStartMin;
  int endMinutes = sleepEndHour * 60 + sleepEndMin;

  return nowMinutes >= startMinutes && nowMinutes < endMinutes;
}

static void fetchTask(void *param)
{
  unsigned long lastDepartureFetch = 0;
  unsigned long lastAnnouncementFetch = 0;
  unsigned long lastWeatherFetch = 0;

  for (;;)
  {
    if (!isNightTime())
    {
      unsigned long now = millis();

      if (lastDepartureFetch == 0 || now - lastDepartureFetch >= shortPollingInterval)
      {
        lastDepartureFetch = now;

        imhr::BusDeparture bus = imhr::fetchBusDeparture();
        imhr::TrainDeparture train = imhr::fetchTrainDeparture();

        xSemaphoreTake(dataMutex, portMAX_DELAY);
        sharedBus = bus;
        sharedTrain = train;
        departureDirty = true;
        xSemaphoreGive(dataMutex);

        Serial.println("----------");
      }

      if (lastAnnouncementFetch == 0 || now - lastAnnouncementFetch >= announcementPollingInterval)
      {
        lastAnnouncementFetch = now;
        Serial.printf("announcement fetch: %lu; free heap space before fetch: %lu\n", now, ESP.getFreeHeap());

        imhr::Announcement announcement = imhr::fetchAnnouncement();

        xSemaphoreTake(dataMutex, portMAX_DELAY);
        sharedAnnouncement = announcement;
        announcementDirty = true;
        xSemaphoreGive(dataMutex);

        Serial.printf("free heap space after fetch: %lu\n", ESP.getFreeHeap());
      }

      if (lastWeatherFetch == 0 || now - lastWeatherFetch >= weatherPollingInterval)
      {
        lastWeatherFetch = now;

        imhr::Weather weather = imhr::fetchWeather();

        xSemaphoreTake(dataMutex, portMAX_DELAY);
        sharedWeather = weather;
        weatherDirty = true;
        xSemaphoreGive(dataMutex);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

static Updates syncSharedData()
{
  Updates updates = {false, false};
  bool hasAnnouncementUpdate = false;
  imhr::Announcement announcement;

  xSemaphoreTake(dataMutex, portMAX_DELAY);
  if (departureDirty)
  {
    shownBus = sharedBus;
    shownTrain = sharedTrain;
    updates.departure = true;
    departureDirty = false;
  }
  if (weatherDirty)
  {
    shownWeather = sharedWeather;
    updates.weather = true;
    weatherDirty = false;
  }
  if (announcementDirty)
  {
    announcement = sharedAnnouncement;
    hasAnnouncementUpdate = true;
    announcementDirty = false;
  }
  xSemaphoreGive(dataMutex);

  if (hasAnnouncementUpdate)
    imhr::setAnnouncementMessage(announcement.hasMessage ? announcement.summary.c_str() : nullptr);

  return updates;
}

static bool minuteChanged()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 0) || timeinfo.tm_min == shownMinute)
    return false;

  shownMinute = timeinfo.tm_min;
  return true;
}

static void renderCurrentView()
{
  if (currentView == View::Departure)
  {
    imhr::displayDeparture(shownBus.line.c_str(), shownBus.minutes, shownTrain.line.c_str(), shownTrain.minutes);
    return;
  }

  struct tm timeinfo;
  bool hasTime = getLocalTime(&timeinfo, 0);
  imhr::displayWeather(shownWeather, hasTime ? &timeinfo : nullptr);
}

void setup()
{
  Serial.begin(115200);
  configTzTime(timeZone, ntpServer);
  imhr::displayInit();
  imhr::buttonInit(viewButtonPin);
  while (!imhr::connectWiFi())
  {
    imhr::displayMessage("!", "wifi problem");
    delay(wifiRetryDelay);
  }
  imhr::displayMessage(":)", "wifi connected!");
  delay(readableMessageDelay);

  dataMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(fetchTask, "fetchTask", 8192, nullptr, 1, nullptr, 0);
}

void loop()
{
  if (isNightTime())
  {
    counter++;
    Serial.println("still sleeping... zZzz... retry counter: " + String(counter));
    imhr::displaySleep();
    // it can cause issues with the wifi connection, when waking up...
    // esp_sleep_enable_timer_wakeup(5 * 60 * 1000000ULL);
    // esp_light_sleep_start();
    delay(30000);
    return;
  }
  counter = 0;
  imhr::displayWake();

  bool viewToggled = imhr::buttonWasPressed();
  if (viewToggled)
    currentView = currentView == View::Departure ? View::Weather : View::Departure;

  Updates updates = syncSharedData();
  bool minuteTicked = minuteChanged();

  bool needsRedraw = currentView == View::Departure
                         ? updates.departure
                         : updates.weather || minuteTicked;

  if (viewToggled || needsRedraw)
    renderCurrentView();

  if (currentView == View::Departure)
    imhr::tickScroll();

  delay(1);
}
