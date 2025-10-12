#include "alarm.h"
#include <Arduino.h>

#define HIGH 440

Alarm alarm;

void setupAlarm(int hour, int minute)
{
    alarm.alarmHour = hour;
    alarm.alarmMinute = minute;
    alarm.isAlarmSet = true;
}

Alarm getAlarm()
{
    return alarm;
}

bool isAlarmTriggered(int currentHour, int currentMinute)
{
    return alarm.isAlarmSet && (currentHour == alarm.alarmHour) && (currentMinute == alarm.alarmMinute);
}

void checkAlarm(Watch currentTime, Alarm alarm, int buzzerPin)
{
    if (isAlarmTriggered(currentTime.hours, currentTime.minutes))
    {
        tone(buzzerPin, HIGH);
    }
    else
    {
        noTone(buzzerPin);
    }
}