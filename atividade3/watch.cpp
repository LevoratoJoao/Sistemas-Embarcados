#include "watch.h"
#include <Arduino.h>

Time watch;
void setupWatch(Time time)
{
    watch = time;
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

Time getWatch()
{
    return watch;
}