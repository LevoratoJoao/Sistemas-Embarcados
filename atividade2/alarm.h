#include "watch.h"
#ifndef ALARM_H
#define ALARM_H

typedef struct
{
    int alarmHour;
    int alarmMinute;
    bool isAlarmSet;
} Alarm;

void setupAlarm(int hour, int minute);
bool isAlarmTriggered(int currentHour, int currentMinute);
Alarm getAlarm();
void checkAlarm(Watch currentTime, Alarm alarm, int buzzerPin);

#endif