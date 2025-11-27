#include "alarm.h"

#define HIGH 440

Time alarm;

void setupAlarm(int hour, int minute, int second)
{
    alarm.hours = hour;
    alarm.minutes = minute;
    alarm.seconds = second;
}

Time getAlarm()
{
    return alarm;
}

bool isAlarmTriggered(int currentHour, int currentMinute, int currentSecond)
{
    return (currentHour == alarm.hours) && (currentMinute == alarm.minutes) && (currentSecond == alarm.seconds);
}