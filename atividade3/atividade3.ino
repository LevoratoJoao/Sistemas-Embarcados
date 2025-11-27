#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>

#include "lcd.h"
#include "watch.h"
#include "alarm.h"
#include "time.h"
#include "joystick.h"

#define ANALOG_X_PIN A0
#define ANALOG_Y_PIN A1
#define ANALOG_BUTTON_PIN A2
#define BUZZER_PIN 6

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Joystick joystick;

Time currentWatch;
Time currentAlarm;

QueueHandle_t xInputQueue;         // Fila para eventos do Joystick/Botão
SemaphoreHandle_t xAlarmSemaphore; // Sinaliza que o alarme disparou
SemaphoreHandle_t xWatchMutex;     // Protege a variável de tempo (currentWatch)

struct InputEvent
{
  JoystickDirection direction;
  bool buttonPressed;
}; // Estrutura para eventos de input

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

volatile States state = NORMAL;

/**
 * @brief Task de leitura do joystick e botão (50ms)
 *
 * @param pvParameters
 */
void TaskInput(void *pvParameters)
{
  (void)pvParameters;

  pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP);
  setupJoystick(&joystick, ANALOG_X_PIN, ANALOG_Y_PIN, ANALOG_BUTTON_PIN);

  for (;;)
  {
    InputEvent event;
    event.direction = isJoystickMoved(&joystick);
    event.buttonPressed = isButtonPressed(&joystick);

    // Se houver qualquer ação, envia para a fila
    if (event.direction != NONE || event.buttonPressed)
    {
      xQueueSend(xInputQueue, &event, portMAX_DELAY);
    }

    vTaskDelay(50 / portTICK_PERIOD_MS); // Polling de 50ms
  }
}

/**
 * @brief Task que atualiza o relógio a cada segundo
 *
 * @param pvParameters
 */
void TaskClock(void *pvParameters)
{
  (void)pvParameters;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 1000 / portTICK_PERIOD_MS; // 1 segundo

  for (;;)
  {
    // Espera exatos 1 segundo desde a última execução
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    // Entra na região crítica para atualizar o relógio
    if (xSemaphoreTake(xWatchMutex, portMAX_DELAY) == pdTRUE)
    {

      // Só atualiza o relógio se NÃO estivermos editando a hora (estado NORMAL ou ALARM_TRIGGERED)
      if (state == NORMAL || state == ALARM_TRIGGERED)
      {
        updateWatch();

        // Verifica se o alarme deve tocar
        Time w = getWatch();
        currentWatch = w;

        if (isAlarmTriggered(w.hours, w.minutes, w.seconds))
        {
          xSemaphoreGive(xAlarmSemaphore); // Dispara a task do buzzer
        }
      }

      xSemaphoreGive(xWatchMutex);
    }
  }
}

/**
 * @brief Consome os inputs e gerencia a Máquina de Estados e o LCD
 *
 * @param pvParameters
 */
void TaskInterface(void *pvParameters)
{
  (void)pvParameters;

  initLCD(&lcd);

  // Configuração Inicial Padrão
  Time t = {12, 0, 0};
  setupWatch(t);
  setupAlarm(12, 1, 0);
  currentWatch = t;
  currentAlarm = getAlarm();

  InputEvent input;

  for (;;)
  {
    // 1. Atualiza a Tela
    // Usamos o Mutex para ler o tempo sem conflito com o TaskClock
    if (xSemaphoreTake(xWatchMutex, 100) == pdTRUE)
    {
      if (state == NORMAL || state == ALARM_TRIGGERED)
      {
        displayTime(&lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);
      }
      else
      {
        // Modo de Edição: Mostra cursor
        // Mapeia o estado para o campo de edição (0 a 5) conforme lcd.cpp
        int editField = 0;
        if (state == SET_HOUR)
        {
          editField = 0;
        }
        else if (state == SET_MINUTE)
        {
          editField = 1;
        }
        else if (state == SET_SECOND)
        {
          editField = 2;
        }
        else if (state == SET_ALARM_HOUR)
        {
          editField = 3;
        }
        else if (state == SET_ALARM_MINUTE)
        {
          editField = 4;
        }
        else if (state == SET_ALARM_SECOND)
        {
          editField = 5;
        }

        displayTimeWithCursor(&lcd, currentWatch, currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds, editField);
      }
      xSemaphoreGive(xWatchMutex);
    }

    // 2. Processar Inputs (Espera por 100ms max, para manter o LCD atualizando)
    if (xQueueReceive(xInputQueue, &input, 100 / portTICK_PERIOD_MS) == pdTRUE)
    {
      // Alarme Tocando -> Botão para
      if (state == ALARM_TRIGGERED && input.buttonPressed)
      {
        state = NORMAL;
        continue;
      }

      // Entrar no modo de edição (Do NORMAL para SET_HOUR)
      if (state == NORMAL && input.buttonPressed)
      {
        state = SET_HOUR;
        continue;
      }

      // Durante edição de hora/alarme
      if (state != NORMAL && state != ALARM_TRIGGERED)
      {

        // Navegação entre os tempos (Esquerda/Direita)
        if (input.direction == RIGHT)
        {
          // Avança estado ou volta pro NORMAL se estiver no fim
          state = (state == SET_ALARM_SECOND) ? SET_HOUR : (States)(state + 1);
        }
        else if (input.direction == LEFT)
        {
          // Volta estado
          state = (state == SET_HOUR) ? SET_ALARM_SECOND : (States)(state - 1);
        }
        // Botão avança para o próximo campo (Confirmar)
        else if (input.buttonPressed)
        {
          if (state == SET_ALARM_SECOND)
          {
            state = NORMAL;
          }
          else
          {
            state = (States)(state + 1);
          }

          // Ao sair de campos de relógio salva
          if (state == SET_ALARM_HOUR)
          {
            xSemaphoreTake(xWatchMutex, portMAX_DELAY);
            setupWatch(currentWatch);
            xSemaphoreGive(xWatchMutex);
          }
          // Ao sair de campos de alarme salva e voltar ao normal
          if (state == NORMAL)
          {
            setupAlarm(currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);
          }
        }
        // escolha de valores de tempo (Cima/Baixo)
        else if (input.direction == UP || input.direction == DOWN)
        {
          int change = (input.direction == UP) ? 1 : -1;

          // Edição do RELÓGIO
          if (state <= SET_SECOND)
          {
            xSemaphoreTake(xWatchMutex, portMAX_DELAY);
            if (state == SET_HOUR)
            {
              currentWatch.hours += change;
              if (currentWatch.hours >= 24)
              {
                currentWatch.hours = 0;
              }
              if (currentWatch.hours < 0)
              {
                currentWatch.hours = 23;
              }
            }
            else if (state == SET_MINUTE)
            {
              currentWatch.minutes += change;
              if (currentWatch.minutes >= 60)
              {
                currentWatch.minutes = 0;
              }
              if (currentWatch.minutes < 0)
              {
                currentWatch.minutes = 59;
              }
            }
            else if (state == SET_SECOND)
            {
              currentWatch.seconds += change;
              if (currentWatch.seconds >= 60)
              {
                currentWatch.seconds = 0;
              }
              if (currentWatch.seconds < 0)
              {
                currentWatch.seconds = 59;
              }
            }
            // Atualiza imediatamente o sistema para refletir a mudança
            setupWatch(currentWatch);
            xSemaphoreGive(xWatchMutex);
          }
          // Edição do ALARME
          else
          {
            if (state == SET_ALARM_HOUR)
            {
              currentAlarm.hours += change;
              if (currentAlarm.hours >= 24)
              {
                currentAlarm.hours = 0;
              }
              if (currentAlarm.hours < 0)
              {
                currentAlarm.hours = 23;
              }
            }
            else if (state == SET_ALARM_MINUTE)
            {
              currentAlarm.minutes += change;
              if (currentAlarm.minutes >= 60)
              {
                currentAlarm.minutes = 0;
              }
              if (currentAlarm.minutes < 0)
              {
                currentAlarm.minutes = 59;
              }
            }
            else if (state == SET_ALARM_SECOND)
            {
              currentAlarm.seconds += change;
              if (currentAlarm.seconds >= 60)
              {
                currentAlarm.seconds = 0;
              }
              if (currentAlarm.seconds < 0)
              {
                currentAlarm.seconds = 59;
              }
            }
            setupAlarm(currentAlarm.hours, currentAlarm.minutes, currentAlarm.seconds);
          }
        }
      }
    }
  }
}

/**
 * @brief Task responsável por tocar o alarme (sirene)
 *
 * @param pvParameters
 */
void TaskAlarm(void *pvParameters)
{
  (void)pvParameters;

  pinMode(BUZZER_PIN, OUTPUT);

  for (;;)
  {
    // Espera o alarme ser disparado
    if (xSemaphoreTake(xAlarmSemaphore, portMAX_DELAY) == pdTRUE)
    {

      state = ALARM_TRIGGERED; // Força estado para interface ver

      // Nova sirene: variação de frequência
      while (state == ALARM_TRIGGERED)
      {
        for (int hz = 440; hz < 1200; hz += 20)
        {
          if (state != ALARM_TRIGGERED)
            break;
          tone(BUZZER_PIN, hz);
          vTaskDelay(20 / portTICK_PERIOD_MS);
        }
        for (int hz = 1200; hz > 440; hz -= 20)
        {
          if (state != ALARM_TRIGGERED)
            break;
          tone(BUZZER_PIN, hz);
          vTaskDelay(20 / portTICK_PERIOD_MS);
        }
      }
      noTone(BUZZER_PIN);
    }
  }
}


void setup()
{
  Serial.begin(115200);

  // Inicializa estruturas do RTOS
  xInputQueue = xQueueCreate(10, sizeof(InputEvent));
  xAlarmSemaphore = xSemaphoreCreateBinary();
  xWatchMutex = xSemaphoreCreateMutex();

  // Criação das Tasks
  xTaskCreate(TaskClock, "Clock", 128, NULL, 4, NULL);         // Máxima prioridade (tempo real)
  xTaskCreate(TaskInput, "Input", 128, NULL, 3, NULL);         // Alta prioridade (responsividade)
  xTaskCreate(TaskAlarm, "Alarm", 128, NULL, 2, NULL);         // Média
  xTaskCreate(TaskInterface, "Interface", 256, NULL, 1, NULL); // Baixa (UI pode esperar ms)

  vTaskStartScheduler();
}

void loop()
{
}