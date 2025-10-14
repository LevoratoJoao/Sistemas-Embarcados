#include "alarm.h"

#define HIGH 440

Time alarm;

void setupAlarm(int hour, int minute)
{
    alarm.hours = hour;
    alarm.minutes = minute;
}

Time getAlarm()
{
    return alarm;
}

bool isAlarmTriggered(int currentHour, int currentMinute)
{
    return (currentHour == alarm.hours) && (currentMinute == alarm.minutes);
}