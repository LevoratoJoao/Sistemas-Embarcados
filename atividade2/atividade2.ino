// These define's must be placed at the beginning before #include "TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG 2
#define _TIMERINTERRUPT_LOGLEVEL_ 0

#define USE_TIMER_1 true

#if (defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__) ||                   \
     defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_MINI) || defined(ARDUINO_AVR_ETHERNET) ||                        \
     defined(ARDUINO_AVR_FIO) || defined(ARDUINO_AVR_BT) || defined(ARDUINO_AVR_LILYPAD) || defined(ARDUINO_AVR_PRO) ||                            \
     defined(ARDUINO_AVR_NG) || defined(ARDUINO_AVR_UNO_WIFI_DEV_ED) || defined(ARDUINO_AVR_DUEMILANOVE) || defined(ARDUINO_AVR_FEATHER328P) ||    \
     defined(ARDUINO_AVR_METRO) || defined(ARDUINO_AVR_PROTRINKET5) || defined(ARDUINO_AVR_PROTRINKET3) || defined(ARDUINO_AVR_PROTRINKET5FTDI) || \
     defined(ARDUINO_AVR_PROTRINKET3FTDI))
#warning Using Timer1
#else
#define USE_TIMER_3 true
#warning Using Timer1, Timer3
#endif

#define TEST_MODE 0

// include the library code:
#include "TimerInterrupt.h"
#include "lcd.h"
#include "watch.h"
#include "alarm.h"
#include "time.h"
#include "joystick.h"

#define ANALOG_X_PIN A0
#define ANALOG_Y_PIN A1
#define ANALOG_BUTTON_PIN A2

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int BUZZER = 6;
Time currentWatch;
Time currentAlarm;
Joystick joystick = {ANALOG_X_PIN, ANALOG_Y_PIN, ANALOG_BUTTON_PIN};

enum States
{
  SET_HOUR,
  SET_MINUTE,
  SET_SECOND,
  SET_ALARM_HOUR,
  SET_ALARM_MINUTE,
  SET_ALARM_SECOND,
  NORMAL,
  ALARM_TRIGGERED
};

States state;
States previousState;

#if USE_TIMER_1

void TimerHandler1()
{
  updateWatch();
  if (isButtonPressed(&joystick))
  {
    state = SET_HOUR;
  }
}

#endif

#define TIMER1_INTERVAL_MS 1000
#define TIMER1_FREQUENCY (float)(1000.0f / TIMER1_INTERVAL_MS)

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP);

  initLCD(&lcd);
  setupJoystick(&joystick, ANALOG_X_PIN, ANALOG_Y_PIN, ANALOG_BUTTON_PIN);
  setupWatch(currentWatch);
  setupAlarm(12, 1, 0);

  state = SET_HOUR;
  previousState = SET_ALARM_SECOND;

  Serial.print(F("\nStarting TimerInterruptTest on "));
  Serial.println(BOARD_TYPE);
  Serial.println(TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = "));
  Serial.print(F_CPU / 1000000);
  Serial.println(F(" MHz"));

#if USE_TIMER_1

  // Timer0 is used for micros(), millis(), delay(), etc and can't be used
  // Select Timer 1-2 for UNO, 0-5 for MEGA
  // Timer 2 is 8-bit timer, only for higher frequency

  ITimer1.init();

  // Using ATmega328 used in UNO => 16MHz CPU clock ,

  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS, TimerHandler1))
  {
    Serial.print(F("Starting  ITimer1 OK, millis() = "));
    Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));

#endif
}

void loop()
{
#if TEST_MODE
  useTestMode(&lcd, state);
#else
  switch (state)
  {
  case SET_HOUR:
    Serial.println("Setting Hour");
    selectHour(currentWatch);
    break;
  case SET_MINUTE:
    Serial.println("Setting Minute");
    selectMinute(currentWatch);
    break;
  case SET_SECOND:
    Serial.println("Setting Second");
    selectSecond(currentWatch);
    setupWatch(currentWatch);
    break;
  case SET_ALARM_HOUR:
    Serial.println("Setting Alarm Hour");
    selectHour(currentAlarm);
    break;
  case SET_ALARM_MINUTE:
    Serial.println("Setting Alarm Minute");
    selectMinute(currentAlarm);
    break;
  case SET_ALARM_SECOND:
    Serial.println("Setting Alarm Second");
    selectSecond(currentAlarm);
    break;
  case NORMAL:
    printTimeInfo();
    delay(1000);

    currentWatch = getWatch();

    bool alarmTriggered = isAlarmTriggered(currentWatch.hours, currentWatch.minutes, currentWatch.seconds);
    Serial.print("Alarm Triggered: ");
    Serial.println(alarmTriggered ? "YES" : "NO");
    if (alarmTriggered)
    {
      triggerAlarm();
      state = NORMAL; // TODO
      break;
    }
  }
  displayTime(&lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);

#endif
}

void printTimeInfo()
{
  Serial.print("Current Time: ");
  Serial.print(currentWatch.hours);
  Serial.print(F(":"));
  Serial.print(currentWatch.minutes);
  Serial.print(F(":"));
  Serial.println(currentWatch.seconds);

  Serial.print("Alarm Time: ");
  Serial.print(currentAlarm.hours);
  Serial.print(F(":"));
  Serial.print(currentAlarm.minutes);
  Serial.print(F(":"));
  Serial.println(currentAlarm.seconds);
}

void selectHour(Time &currentTime)
{
  while (true)
  {
    displayTimeWithCursor(&lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds, state == SET_HOUR ? 0 : 3);

    JoystickDirection direction = isJoystickMoved(&joystick);
    switch (direction)
    {
    case UP:
      currentTime.hours++;
      if (currentTime.hours >= 24)
      {
        currentTime.hours = 0;
      }
      break;
    case DOWN:
      currentTime.hours--;
      if (currentTime.hours < 0)
      {
        currentTime.hours = 23;
      }
      break;
    case LEFT:
      state = (States)((state == SET_HOUR) ? SET_ALARM_SECOND : state - 1);
      return;
    case RIGHT:
      state = (States)((state == SET_ALARM_SECOND) ? SET_HOUR : state + 1);
      return;
    default:
      break;
    }

    if (isButtonPressed(&joystick))
    {
      previousState = state;
      state = state == SET_HOUR ? SET_MINUTE : SET_ALARM_MINUTE;
      break;
    }
    delay(50);
  }

  Serial.print("Hour selected: ");
  Serial.println(currentTime.hours);
}

void selectMinute(Time &currentTime)
{
  while (true)
  {
    displayTimeWithCursor(&lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds, state == SET_MINUTE ? 1 : 4);

    JoystickDirection direction = isJoystickMoved(&joystick);

    switch (direction)
    {
    case UP:
      currentTime.minutes++;
      if (currentTime.minutes >= 60)
      {
        currentTime.minutes = 0;
      }
      break;
    case DOWN:
      currentTime.minutes--;
      if (currentTime.minutes < 0)
      {
        currentTime.minutes = 59;
      }
      break;
    case LEFT:
      state = (States)((state == SET_HOUR) ? SET_ALARM_SECOND : state - 1);
      return;
    case RIGHT:
      state = (States)((state == SET_ALARM_SECOND) ? SET_HOUR : state + 1);
      return;
    default:
      break;
    }

    if (isButtonPressed(&joystick))
    {
      previousState = state;
      state = state == SET_MINUTE ? SET_SECOND : SET_ALARM_SECOND;
      break;
    }
    delay(50);
  }
  Serial.print("Minute selected: ");
  Serial.println(currentTime.minutes);
}

void selectSecond(Time &currentTime)
{
  while (true)
  {
    displayTimeWithCursor(&lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds, state == SET_SECOND ? 2 : 5);

    JoystickDirection direction = isJoystickMoved(&joystick);
    switch (direction)
    {
    case UP:
      currentTime.seconds++;
      if (currentTime.seconds >= 60)
      {
        currentTime.seconds = 0;
      }
      break;
    case DOWN:
      currentTime.seconds--;
      if (currentTime.seconds < 0)
      {
        currentTime.seconds = 59;
      }
      break;
    case LEFT:
      state = (States)((state == SET_HOUR) ? SET_ALARM_SECOND : state - 1);
      return;
    case RIGHT:
      state = (States)((state == SET_ALARM_SECOND) ? SET_HOUR : state + 1);
      return;
    default:
      break;
    }

    if (isButtonPressed(&joystick))
    {
      state == SET_SECOND ? setupWatch(currentWatch) : setupAlarm(currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);
      previousState = state;
      state = state == SET_SECOND ? SET_ALARM_HOUR : NORMAL;
      break;
    }
    delay(50);
  }
  Serial.print("Second selected: ");
  Serial.println(currentTime.seconds);
}

void triggerAlarm()
{
  Serial.println("Triggering Alarm!");
  tone(BUZZER, 440);
  unsigned long startTime = millis();
  while (true)
  {
    displayTime(&lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);
    delay(200);
    if (digitalRead(ANALOG_BUTTON_PIN) == 0)
    {
      while (digitalRead(ANALOG_BUTTON_PIN) == 0)
      {
        delay(10);
      }
      noTone(BUZZER);
      break;
    }
  }
  noTone(BUZZER);
  Serial.println("Alarm Stopped");
}

void useTestMode(LiquidCrystal *lcd, States state)
{
  switch (state)
  {
  case SET_HOUR:
    currentWatch.hours = 23;
    state = SET_MINUTE;
    break;
  case SET_MINUTE:
    currentWatch.minutes = 58;
    state = SET_SECOND;
    break;
  case SET_SECOND:
    currentWatch.seconds = 50;
    setupWatch(currentWatch);
    state = SET_ALARM_HOUR;
    break;
  case SET_ALARM_HOUR:
    currentAlarm.hours = 23;
    state = SET_ALARM_MINUTE;
    break;
  case SET_ALARM_MINUTE:
    currentAlarm.minutes = 59;
    state = SET_ALARM_SECOND;
    break;
  case SET_ALARM_SECOND:
    currentAlarm.seconds = 1;
    setupAlarm(currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);
    state = NORMAL;
    break;
  case NORMAL:
    printTimeInfo();
    delay(1000);

    currentWatch = getWatch();

    bool alarmTriggered = isAlarmTriggered(currentWatch.hours, currentWatch.minutes, currentWatch.seconds);
    if (alarmTriggered)
    {
      state = ALARM_TRIGGERED;
      break;
    }
    else
    {
      displayTime(lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);
      break;
    }
  case ALARM_TRIGGERED:
    triggerAlarm();
    state = NORMAL;
    break;
  }
}