#ifndef LCD_H
#define LCD_H

#include "LiquidCrystal.h"

typedef struct
{
    int currentState;
    int hour;
    int minute;
    int second;
    int lcdState;
} LCD;

void initLCD(LiquidCrystal *lcd);
void updateLCD(LiquidCrystal *lcd, int hour, int minute, int second);
void displayTime(LiquidCrystal *lcd, int hour, int minute, int second, int alarmHour, int alarmMinute);

#endif