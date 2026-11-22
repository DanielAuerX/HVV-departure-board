#pragma once

namespace imhr
{

    class Debouncer
    {
    public:
        bool update(bool rawPressed, unsigned long nowMs);

    private:
        static const unsigned long debounceMs = 30;
        bool stablePressed = false;
        bool lastRawPressed = false;
        unsigned long lastChangeMs = 0;
    };

}
