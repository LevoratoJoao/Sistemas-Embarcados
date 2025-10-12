#include "lcd.h"

void initLCD(LiquidCrystal *lcd)
{
    lcd->begin(16, 2);
    // Print a message to the LCD.
    lcd->print("hello, world!");
}

void updateLCD(LiquidCrystal *lcd, int hour, int minute, int second)
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Time:");
    lcd->setCursor(0, 1);
    if (hour < 10)
        lcd->print("0");
    lcd->print(hour);
    lcd->print(":");
    if (minute < 10)
        lcd->print("0");
    lcd->print(minute);
    lcd->print(":");
    if (second < 10)
        lcd->print("0");
    lcd->print(second);
}

void displayTime(LiquidCrystal *lcd, int hour, int minute, int second, int alarmHour, int alarmMinute)
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Time:");
    if (hour < 10)
    {
        lcd->print("0");
    }
    lcd->print(hour);
    lcd->print(":");
    if (minute < 10)
    {
        lcd->print("0");
    }
    lcd->print(minute);
    lcd->print(":");
    if (second < 10)
    {
        lcd->print("0");
    }
    lcd->print(second);

    lcd->setCursor(0, 1);
    lcd->print("Alarm:");
    if (hour < 10)
    {
        lcd->print("0");
    }
    lcd->print(alarmHour);
    lcd->print(":");
    if (alarmMinute < 10)
    {
        lcd->print("0");
    }
    lcd->print(alarmMinute);
}