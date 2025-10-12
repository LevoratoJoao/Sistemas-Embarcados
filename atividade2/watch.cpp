#include "watch.h"
#include <Arduino.h>

Watch watch;
void setupWatch(int hour, int minute, int second)
{
    watch.hours = hour;
    watch.minutes = minute;
    watch.seconds = second;
}

void updateWatch()
{
    watch.seconds++;
    if (watch.seconds >= 60)
    {
        watch.seconds = 0;
        watch.minutes++;
        if (watch.minutes >= 60)
        {
            watch.minutes = 0;
            watch.hours++;
            if (watch.hours >= 24)
            {
                watch.hours = 0;
            }
        }
    }
}

Watch getWatch()
{
    return watch;
}