#include "imhr_display.h"
#include "imhr_display_logic.h"

namespace imhr
{

    static String scrollMessage = "";
    static int scrollOffset = 0;
    static unsigned long lastScrollUpdate = 0;
    static const unsigned long scrollIntervalMs = 80; // lower = faster scroll
    static const int scrollResetGapPx = 20;           // gap before message repeats

    U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

    void displayInit()
    {
        display.begin();
    }

    void displaySleep()
    {
        display.setPowerSave(1);
    }

    void displayWake()
    {
        display.setPowerSave(0);
    }

    static void drawCenteredStr(const char *text, int y)
    {
        int width = display.getUTF8Width(text);
        display.drawUTF8((128 - width) / 2, y, text);
    }

    void displayMessage(const char *line1, const char *line2)
    {
        display.clearBuffer();

        display.setFont(u8g2_font_ncenB14_tf);
        drawCenteredStr(line1, 30);

        display.setFont(u8g2_font_ncenB08_tf);
        drawCenteredStr(line2, 50);

        display.sendBuffer();
    }

    static void drawDepartureLine(const char *label, const char *minsStr, int y)
    {
        display.setFont(u8g2_font_logisoso16_tf);
        display.drawUTF8(0, y, label);

        int strW = display.getUTF8Width(minsStr);
        display.drawUTF8(128 - strW, y, minsStr);
    }

    void displayDeparture(const char *busLine, int busMins,
                          const char *trainPlatform, int trainMins)
    {
        char busLabel[16];
        char trainLabel[16];
        snprintf(busLabel, sizeof(busLabel), "%s", busLine);

        if (trainPlatform && strlen(trainPlatform) > 0)
            snprintf(trainLabel, sizeof(trainLabel), trainPlatform);
        else
            snprintf(trainLabel, sizeof(trainLabel), "Zug");

        char busMinsStr[12];
        char trainMinsStr[12];
        determineMinuteDisplay(busMinsStr, sizeof(busMinsStr), busMins);
        determineMinuteDisplay(trainMinsStr, sizeof(trainMinsStr), trainMins);

        display.clearBuffer();
        drawDepartureLine(busLabel, busMinsStr, 20);
        drawDepartureLine(trainLabel, trainMinsStr, 48);
        display.sendBuffer();
    }

    static void drawRow(const uint8_t *leftFont, const char *left, const uint8_t *rightFont, const char *right, int y)
    {
        display.setFont(leftFont);
        display.drawUTF8(0, y, left);

        display.setFont(rightFont);
        int strW = display.getUTF8Width(right);
        display.drawUTF8(128 - strW, y, right);
    }

    void displayWeather(const Weather &weather, const tm *time)
    {
        char clock[6] = "--:--";
        char date[12] = "";
        if (time)
        {
            strftime(clock, sizeof(clock), "%H:%M", time);
            formatDate(date, sizeof(date), *time);
        }

        char temperature[8] = "--";
        char minimum[8];
        char maximum[8];
        char range[24] = "";
        char rain[16] = "";
        if (weather.valid)
        {
            formatTemperature(temperature, sizeof(temperature), weather.temperature);
            formatTemperature(minimum, sizeof(minimum), weather.temperatureMin);
            formatTemperature(maximum, sizeof(maximum), weather.temperatureMax);
            snprintf(range, sizeof(range), "%s bis %s", minimum, maximum);
            snprintf(rain, sizeof(rain), "Regen %d%%", weather.precipitationProbability);
        }

        const char *condition = weather.valid ? conditionLabel(weather.condition) : "";

        display.clearBuffer();
        drawRow(u8g2_font_logisoso16_tf, clock, u8g2_font_ncenB08_tf, date, 20);
        drawRow(u8g2_font_logisoso16_tf, temperature, u8g2_font_ncenB08_tf, condition, 42);
        drawRow(u8g2_font_ncenB08_tf, range, u8g2_font_ncenB08_tf, rain, 62);
        display.sendBuffer();
    }

    void setAnnouncementMessage(const char *message)
    {
        String newMessage = message ? message : "";
        if (newMessage != scrollMessage)
        {
            scrollMessage = newMessage;
            scrollOffset = 0; // only reset when the text actually changes
        }
    }
    
    void tickScroll()
    {
        if (scrollMessage.isEmpty())
            return;

        unsigned long now = millis();
        if (now - lastScrollUpdate < scrollIntervalMs)
            return;
        lastScrollUpdate = now;

        display.setFont(u8g2_font_5x7_tf);
        int textWidth = display.getUTF8Width(scrollMessage.c_str());

        scrollOffset++;
        if (scrollOffset > textWidth + scrollResetGapPx)
            scrollOffset = 0;

        // redraw only the bottom row, leave bus/train lines untouched
        display.setDrawColor(0);
        display.drawBox(0, 56, 128, 8); // clear bottom row
        display.setDrawColor(1);
        display.drawUTF8(128 - scrollOffset, 62, scrollMessage.c_str());
        display.sendBuffer();
    }

}
