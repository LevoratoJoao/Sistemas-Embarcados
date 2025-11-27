#include "time.h"
#ifndef LCD_H
#define LCD_H

#include "LiquidCrystal.h"

void initLCD(LiquidCrystal *lcd);
void updateLCD(LiquidCrystal *lcd, Time time);
// TODO: mudar alarm para Time
void displayTime(LiquidCrystal *lcd, Time time, int alarmHour, int alarmMinute, int alarmSecond);
void displayTimeWithCursor(LiquidCrystal *lcd, Time time, int alarmHour, int alarmMinute, int alarmSecond, int editField);

#endif