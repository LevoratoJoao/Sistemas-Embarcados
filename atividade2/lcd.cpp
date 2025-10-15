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

void displayTime(LiquidCrystal *lcd, Time time, int alarmHour, int alarmMinute, int alarmSecond)
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
    lcd->print(":");
    if (alarmSecond < 10)
    {
        lcd->print("0");
    }
    lcd->print(alarmSecond);
}

void displayTimeWithCursor(LiquidCrystal *lcd, Time time, int alarmHour, int alarmMinute, int alarmSecond, int editField)
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Time:");

    // Hour
    if (editField == 0)
        lcd->print(">");
    if (time.hours < 10)
        lcd->print("0");
    lcd->print(time.hours);

    lcd->print(":");

    // Minute
    if (editField == 1)
        lcd->print(">");
    if (time.minutes < 10)
        lcd->print("0");
    lcd->print(time.minutes);

    lcd->print(":");

    // Second
    if (editField == 2)
        lcd->print(">");
    if (time.seconds < 10)
        lcd->print("0");
    lcd->print(time.seconds);

    // Alarm line
    lcd->setCursor(0, 1);
    lcd->print("Alarm:");

    // Alarm Hour
    if (editField == 3)
        lcd->print(">");
    if (alarmHour < 10)
        lcd->print("0");
    lcd->print(alarmHour);

    lcd->print(":");

    // Alarm Minute
    if (editField == 4)
        lcd->print(">");
    if (alarmMinute < 10)
        lcd->print("0");
    lcd->print(alarmMinute);

    lcd->print(":");

    // Alarm Second
    if (editField == 5)
        lcd->print(">");
    if (alarmSecond < 10)
        lcd->print("0");
    lcd->print(alarmSecond);
}
