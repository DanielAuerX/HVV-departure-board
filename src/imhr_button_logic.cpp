#include "imhr_button_logic.h"

namespace imhr
{

    bool Debouncer::update(bool rawPressed, unsigned long nowMs)
    {
        if (rawPressed != lastRawPressed)
        {
            lastRawPressed = rawPressed;
            lastChangeMs = nowMs;
            return false;
        }

        if (nowMs - lastChangeMs < debounceMs || rawPressed == stablePressed)
            return false;

        stablePressed = rawPressed;
        return stablePressed;
    }

}
