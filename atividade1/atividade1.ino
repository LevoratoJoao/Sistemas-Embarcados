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

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "TimerInterrupt.h"
#include "Communication.h"

#define TX 18
#define RX 19

volatile bool shouldSendData = false;
volatile bool shouldReceiveData = false;
Communication comms;

#if USE_TIMER_1

void TimerHandler()
{
  sendData(&comms, "Hello Marcelo!");
}

#endif

#if (USE_TIMER_3)

void TimerHandler1()
{
  receiveData(&comms);
}

#endif

#define USING_LOOP_TEST false

#define TIMER1_INTERVAL_MS 2000
#define TIMER1_FREQUENCY (float)(1000.0f / TIMER1_INTERVAL_MS)

#define TIMER_INTERVAL_MS 2000
#define TIMER_FREQUENCY (float)(1000.0f / TIMER_INTERVAL_MS)

#if USING_LOOP_TEST
#define TIMER1_DURATION_MS (10UL * TIMER1_INTERVAL_MS)
#define TIMER_DURATION_MS (20UL * TIMER_INTERVAL_MS)
#else
#define TIMER1_DURATION_MS 0
#define TIMER_DURATION_MS 0
#endif

void setup()
{

  Serial.begin(115200);
  while (!Serial)
    ;

  setupCommunication(&comms, TX, RX);

  Serial.print(F("\nStarting TimerInterruptTest on "));
  Serial.println(BOARD_TYPE);
  Serial.println(TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = "));
  Serial.print(F_CPU / 1000000);
  Serial.println(F(" MHz"));

  ITimer1.init();

  if (ITimer1.attachInterruptInterval(TIMER_INTERVAL_MS, TimerHandler))
  {
    Serial.print(F("Starting  ITimer1 OK, millis() = "));
    Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));

  ITimer3.init();

  if (ITimer3.attachInterruptInterval(TIMER1_INTERVAL_MS, TimerHandler1))
  {
    Serial.print(F("Starting  ITimer3 OK, millis() = "));
    Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer3. Select another freq. or timer"));
}

void loop()
{
}