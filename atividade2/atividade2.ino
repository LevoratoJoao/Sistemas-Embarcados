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

// include the library code:
#include "TimerInterrupt.h"
#include "lcd.h"
#include "watch.h"
#include "alarm.h"

#define ANALOG_X_PIN A0
#define ANALOG_Y_PIN A1
#define ANALOG_BUTTON_PIN A2

#if USE_TIMER_1

void TimerHandler1()
{
  Serial.println("Timer1 Interrupt");
  updateWatch();
}

#endif

#define TIMER1_INTERVAL_MS 1000
#define TIMER1_FREQUENCY (float)(1000.0f / TIMER1_INTERVAL_MS)

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int BUZZER = 6;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  initLCD(&lcd);
  setupWatch(12, 0, 0);
  setupAlarm(12, 1);

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
  Watch currentWatch = getWatch();
  Serial.print(F("Current Time: "));
  Serial.print(currentWatch.hours);
  Serial.print(F(":"));
  Serial.print(currentWatch.minutes);
  Serial.print(F(":"));
  Serial.println(currentWatch.seconds);
  delay(1000);

  Alarm currentAlarm = getAlarm();
  Serial.print(F("Alarm Time: "));
  Serial.print(currentAlarm.alarmHour);
  Serial.print(F(":"));
  Serial.print(currentAlarm.alarmMinute);
  displayTime(&lcd, currentWatch.hours, currentWatch.minutes, currentWatch.seconds, currentAlarm.alarmHour, currentAlarm.alarmMinute);
  checkAlarm(currentWatch, currentAlarm, BUZZER);
}
