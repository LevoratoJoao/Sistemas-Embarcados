#include "watch.h"
#include "time.h"
#ifndef ALARM_H
#define ALARM_H

void setupAlarm(int hour, int minute, int second);
bool isAlarmTriggered(int currentHour, int currentMinute, int currentSecond);
Time getAlarm();

#endif