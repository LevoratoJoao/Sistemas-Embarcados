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
#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <semphr.h>
#include "TimerInterrupt.h"
#include "lcd.h"
#include "watch.h"
#include "alarm.h"
#include "time.h"
#include "joystick.h"

#define ANALOG_X_PIN A0
#define ANALOG_Y_PIN A1
#define ANALOG_BUTTON_PIN A2

void task_LCD(void *pvParameters);
void task_watch(void *pvParameters);
void task_alarm(void *pvParameters);
void task_joystick(void *pvParameters);

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int BUZZER = 6;
Time currentWatch;
Time currentAlarm;
Joystick joystick = {ANALOG_X_PIN, ANALOG_Y_PIN, ANALOG_BUTTON_PIN};

SemaphoreHandle_t xSerial_semaphore;
QueueHandle_t xQueue_LCD, xQueue_watch, xQueue_alarm, xQueue_joystick, xQueue_state;

enum States
{
  SET_HOUR,
  SET_MINUTE,
  SET_SECOND,
  SET_ALARM_HOUR,
  SET_ALARM_MINUTE,
  SET_ALARM_SECOND,
  NORMAL
};

States state;
States previousState;

#if USE_TIMER_1

void TimerHandler1()
{
  Serial.println("Timer Interrupt 1 Triggered");

  // Always update watch
  updateWatch();
  currentWatch = getWatch();
  xQueueOverwriteFromISR(xQueue_watch, &currentWatch, NULL);

  // Check for button press to enter edit mode
  if (isButtonPressed(&joystick))
  {
    Serial.println("Button pressed in Timer");
    States newState = SET_HOUR;
    xQueueOverwriteFromISR(xQueue_state, &newState, NULL);
  }

  // Check alarm trigger
  if (isAlarmTriggered(currentWatch.hours, currentWatch.minutes, currentWatch.seconds))
  {
    xQueueOverwriteFromISR(xQueue_alarm, &currentWatch, NULL);
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

  xQueue_LCD = xQueueCreate(1, sizeof(LiquidCrystal *));
  xQueue_watch = xQueueCreate(1, sizeof(Time));
  xQueue_alarm = xQueueCreate(1, sizeof(Time));
  xQueue_joystick = xQueueCreate(1, sizeof(Joystick));
  xQueue_state = xQueueCreate(1, sizeof(States));

  xSerial_semaphore = xSemaphoreCreateMutex();
  if (xSerial_semaphore == NULL)
  {
    Serial.println("Erro: nao e possivel criar o semaforo");
    while (1)
      ;
  }

  xTaskCreate(
      state_machine_task,
      "State Machine Task",
      256,
      NULL,
      10,
      NULL);

  xTaskCreate(
      task_LCD,
      "LCD Task",
      256,
      NULL,
      5,
      NULL);

  xTaskCreate(
      task_watch,
      "Watch Task",
      256,
      NULL,
      1,
      NULL);

  xTaskCreate(
      task_alarm,
      "Alarm Task",
      256,
      NULL,
      1,
      NULL);

  xTaskCreate(
      task_joystick,
      "Joystick Task",
      256,
      NULL,
      1,
      NULL);

  initLCD(&lcd);
  setupJoystick(&joystick, ANALOG_X_PIN, ANALOG_Y_PIN, ANALOG_BUTTON_PIN);
  setupWatch(currentWatch);
  setupAlarm(12, 1, 0);

  state = SET_HOUR;
  xQueueOverwrite(xQueue_state, (void *)&state);

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
    Joystick js;
    if (xQueueReceive(xQueue_joystick, &js, portMAX_DELAY) == pdPASS)
    {
      JoystickDirection direction = isJoystickMoved(&joystick);
      displayTimeWithCursor(&lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds, state == SET_HOUR ? 0 : 3);

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
        xQueueOverwrite(xQueue_state, (void *)&state);
        return;
      case RIGHT:
        state = (States)((state == SET_ALARM_SECOND) ? SET_HOUR : state + 1);
        xQueueOverwrite(xQueue_state, (void *)&state);
        return;
      default:
        break;
      }
      if (isButtonPressed(&joystick))
      {
        state = state == SET_HOUR ? SET_MINUTE : SET_ALARM_MINUTE;
        xQueueOverwrite(xQueue_state, (void *)&state);
        return;
      }
    }
  }
}

void selectMinute(Time &currentTime)
{
  while (true)
  {
    if (xQueueReceive(xQueue_joystick, &joystick, portMAX_DELAY) == pdPASS)
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
        xQueueOverwrite(xQueue_state, (void *)&state);
        return;
      case RIGHT:
        state = (States)((state == SET_ALARM_SECOND) ? SET_HOUR : state + 1);
        xQueueOverwrite(xQueue_state, (void *)&state);
        return;
      default:
        break;
      }

      if (isButtonPressed(&joystick))
      {
        state = state == SET_MINUTE ? SET_SECOND : SET_ALARM_SECOND;
        xQueueOverwrite(xQueue_state, (void *)&state);
        break;
      }
    }
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

void task_LCD(void *pvParameters)
{
  LiquidCrystal *lcdPtr;
  for (;;)
  {
    if (xQueueReceive(xQueue_LCD, &lcdPtr, portMAX_DELAY) == pdPASS && xQueueReceive(xQueue_watch, &currentWatch, portMAX_DELAY) == pdPASS &&
        xQueueReceive(xQueue_alarm, &currentAlarm, portMAX_DELAY) == pdPASS)
    {
      displayTime(lcdPtr, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void task_watch(void *pvParameters)
{
  for (;;)
  {
    currentWatch = getWatch();
    xQueueSend(xQueue_watch, &currentWatch, portMAX_DELAY);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void task_alarm(void *pvParameters)
{
  for (;;)
  {
    xQueueSend(xQueue_alarm, &currentAlarm, portMAX_DELAY);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void task_joystick(void *pvParameters)
{
  for (;;)
  {
    xQueueSend(xQueue_joystick, &joystick, portMAX_DELAY);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void state_machine_task(void *pvParameters)
{
  for (;;)
  {

    Serial.println("State Machine Task Running");
    switch (state)
    {
    case SET_HOUR:
      Serial.println("Setting Hour");
      selectHour(currentWatch);
      Serial.print("Back to menu");
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
    case NORMAL: // TODO: timeout pra voltar pro normal
      if (xQueueReceive(xQueue_watch, &currentWatch, portMAX_DELAY) == pdPASS &&
          xQueueReceive(xQueue_alarm, &currentAlarm, portMAX_DELAY) == pdPASS)
      {

        printTimeInfo();

        bool alarmTriggered = isAlarmTriggered(currentWatch.hours, currentWatch.minutes, currentWatch.seconds);
        Serial.print("Alarm Triggered: ");
        Serial.println(alarmTriggered ? "YES" : "NO");
        if (alarmTriggered)
        {
          triggerAlarm();
          state = NORMAL;
          vTaskDelay(100 / portTICK_PERIOD_MS);
          xQueueSend(xQueue_state, &state, portMAX_DELAY);
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        xQueueSend(xQueue_state, &state, portMAX_DELAY);
        xQueueSend(xQueue_joystick, &joystick, portMAX_DELAY);
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      break;
    default:
      Serial.println("Unknown State");
      state = NORMAL;
      vTaskDelay(100 / portTICK_PERIOD_MS);
      xQueueSend(xQueue_state, &state, portMAX_DELAY);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    }
    // if (xQueueReceive(xQueue_LCD, &lcd, portMAX_DELAY) == pdPASS)
    // {
    //   switch (state)
    //   {
    //   case SET_HOUR:
    //   case SET_MINUTE:
    //   case SET_SECOND:
    //   case SET_ALARM_HOUR:
    //   case SET_ALARM_MINUTE:
    //   case SET_ALARM_SECOND:
    //     displayTimeWithCursor(
    //         &lcd,
    //         currentWatch,
    //         currentAlarm.hours,
    //         currentAlarm.minutes,
    //         currentAlarm.seconds,
    //         (state == SET_HOUR || state == SET_ALARM_HOUR)       ? 0
    //         : (state == SET_MINUTE || state == SET_ALARM_MINUTE) ? 1
    //                                                              : 2);
    //     break;
    //   case NORMAL:
    //   default:
    //     displayTime(&lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);
    //     break;
    //   }
    // }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}