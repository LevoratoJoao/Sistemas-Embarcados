#include "lcd.h"

void initLCD(LiquidCrystal *lcd)
{
    lcd->begin(16, 2);
    // Print a message to the LCD.
    lcd->print("hello, world!");
}

void updateLCD(LiquidCrystal *lcd, Time time)
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Time:");
    lcd->setCursor(0, 1);
    if (time.hours < 10)
        lcd->print("0");
    lcd->print(time.hours);
    lcd->print(":");
    if (time.minutes < 10)
        lcd->print("0");
    lcd->print(time.minutes);
    lcd->print(":");
    if (time.seconds < 10)
        lcd->print("0");
    lcd->print(time.seconds);
}

void displayTime(LiquidCrystal *lcd, Time time, int alarmHour, int alarmMinute)
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Time:");
    if (time.hours < 10)
    {
        lcd->print("0");
    }
    lcd->print(time.hours);
    lcd->print(":");
    if (time.minutes < 10)
    {
        lcd->print("0");
    }
    lcd->print(time.minutes);
    lcd->print(":");
    if (time.seconds < 10)
    {
        lcd->print("0");
    }
    lcd->print(time.seconds);

    lcd->setCursor(0, 1);
    lcd->print("Alarm:");
    if (alarmHour < 10)
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