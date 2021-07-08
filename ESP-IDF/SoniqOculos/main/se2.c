#include "main.h"

//* 7

void LED_Toggle();

void task(void *arg)
{
    TickType_t tick = xTaskGetTickCount();
    for (;;)
    {
        vTaskDelayUntil(&tick, 200);
        LED_Toggle();
    }
}

//* 8

#define WAIT_120 120000 // ms
#define WAIT_180 180000 // ms

enum
{
    IDLE,
    SLOW,
    FAST
};

SemaphoreHandle_t semaphore;
void Motor(int on, int vel);

/**
 * @brief Será necessário um semaphore de maneira a comunicar com a task que lida com o botão para saber quando é que este foi pressionado.
 * Espera-se que o semaphore esteja livre, caso falhe (timeout) volta para o estado IDLE, caso pdTRUE é porque o botão foi clicado e volta a libertar o semaphore imediatamente a seguir
 */
void MotorControl(void *arg)
{
    int state = IDLE;
    for (;;)
    {
        /*Motor(0, 0);

        xSemaphoreTake(semaphore, portMAX_DELAY);
        Motor(1, 0);
        xSemaphoreGive(semaphore);

        if (!xSemaphoreTake(semaphore, WAIT_120))
            continue;
        Motor(1, 1);
        xSemaphoreGive(semaphore);

        if(xSemaphoreTake(semaphore, WAIT_180))
            xSemaphoreGive(semaphore);*/

        switch (state)
        {
        case IDLE:
            Motor(0, 0);
            xSemaphoreTake(semaphore, portMAX_DELAY);
            xSemaphoreGive(semaphore);
            state = SLOW;
            break;

        case SLOW:
            Motor(1, 0);
            if (xSemaphoreTake(semaphore, WAIT_120))
            {
                state = FAST;
                xSemaphoreGive(semaphore);
            }
            else
                state = IDLE;
            break;

        case FAST:
            Motor(1, 1);
            if (xSemaphoreTake(semaphore, WAIT_180))
                xSemaphoreGive(semaphore);
            state = IDLE;
            break;
        }
    }
}

#define DEBOUNCE 10 // ms

bool GetButtonState();
bool GetButtonEvent();

void Button(void *arg)
{
    for (;;)
    {
        vTaskDelay(DEBOUNCE);
        if (GetButtonState() && GetButtonEvent()) // Button pressed
        {
            xSemaphoreGive(semaphore);
            xSemaphoreTake(semaphore, portMAX_DELAY);
        }
    }
}

//* 8

#define BTN 1 << 10

#define NONE 0
#define SHORT_PRESS 1
#define LONG_PRESS 2

typedef struct
{
    int FIODIR;
    int FIOPIN;
    int FIOSET;
    int FIOCLR;
} LPC;
LPC *LPC_GPIO2;

QueueHandle_t queue;

/**
 * @brief É necessária uma queue para enviar a informação sobre o código de operação. Usa-se o xTaskGetTickCount() para obter o número de ticks até ao momento e comparar com o registado quando o botão é pressionado
 */
void BTN_Control(void *arg)
{
    LPC_GPIO2->FIODIR &= ~BTN; // Input
    TickType_t tick = xTaskGetTickCount();
    int press;
    for (;;)
    {
        delay(DEBOUNCE);
        if (~LPC_GPIO2->FIOPIN & BTN)
        {
            if (press != NONE)
            {
                tick = xTaskGetTickCount();
                press = NONE;
            }

            if (xTaskGetTickCount() - tick > 1000 && press != LONG_PRESS)
            {
                press = LONG_PRESS;
                xQueueSend(queue, &press, portMAX_DELAY);
            }
        }
        else
        {
            if (xTaskGetTickCount() - tick < 500)
            {
                press = SHORT_PRESS;
                xQueueSend(queue, &press, portMAX_DELAY);
            }
        }
    }
}

void VALV_SetValue(int value);

void VALV_Control(void *arg)
{
    TickType_t tick = xTaskGetTickCount();
    int press;
    for (;;)
    {
        if (!xQueueReceive(queue, &press, 3000) && press == LONG_PRESS)
        {
            VALV_SetValue(0);
            press = NONE;
        }

        switch (press)
        {
        case LONG_PRESS:
            VALV_SetValue(1);
            break;

        case SHORT_PRESS:
            VALV_SetValue(0);
            break;

        default:
            break;
        }
    }
}
