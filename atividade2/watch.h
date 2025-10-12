#ifndef WATCH_H
#define WATCH_H

typedef struct
{
    int hours;
    int minutes;
    int seconds;
} Watch;

void setupWatch(int hour, int minute, int second);
void updateWatch();
Watch getWatch();

#endif