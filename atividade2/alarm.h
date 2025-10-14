#include "watch.h"
#include "time.h"
#ifndef ALARM_H
#define ALARM_H



void setupAlarm(int hour, int minute);
bool isAlarmTriggered(int currentHour, int currentMinute);
Time getAlarm();

#endif