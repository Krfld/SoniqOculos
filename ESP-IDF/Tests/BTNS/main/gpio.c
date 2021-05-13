#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define HOLD_TIME 2 // s

#define DEBOUNCE 50 // ms

#define BUTTON_1 GPIO_NUM_13
#define BUTTON_2 GPIO_NUM_12
#define BUTTON_3 GPIO_NUM_14

static xTaskHandle s_gpio_task_handle = NULL;
static xTaskHandle s_hold_task_handle = NULL;

static void gpio_task_handler(void *arg);
void gpio_task_start_up(void);
void gpio_task_shut_down(void);

void app_main(void)
{
    gpio_task_start_up();
}

static void hold_task(void *arg)
{
    vTaskDelay(HOLD_TIME * 1000 / portTICK_PERIOD_MS);

    if (gpio_get_level(BUTTON_1))
        printf("Shutting down...\n");

    s_hold_task_handle = NULL;
    vTaskDelete(NULL);
}

static void gpio_task_handler(void *arg)
{

    bool buttons_state[3] = {false, false, false};

    gpio_pad_select_gpio(BUTTON_1);
    gpio_set_direction(BUTTON_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_1, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(BUTTON_2);
    gpio_set_direction(BUTTON_2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_2, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(BUTTON_3);
    gpio_set_direction(BUTTON_3, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_3, GPIO_PULLDOWN_ONLY);

    for (;;)
    {
        vTaskDelay(DEBOUNCE / portTICK_PERIOD_MS);

        if (gpio_get_level(BUTTON_1) != buttons_state[0])
        {
            buttons_state[0] = !buttons_state[0];
            if (buttons_state[0])
            {
                printf("Clicked BUTTON 1\n");
                if (!s_hold_task_handle)
                    xTaskCreate(hold_task, "hold_task", 2 * 1024, (void *)BUTTON_1, 10, &s_hold_task_handle);
            }
            else
            {
                printf("Released BUTTON 1\n");
                if (s_hold_task_handle)
                {
                    vTaskDelete(s_hold_task_handle);
                    s_hold_task_handle = NULL;
                }
            }
        }

        if (gpio_get_level(BUTTON_2) != buttons_state[1])
        {
            buttons_state[1] = !buttons_state[1];
            if (buttons_state[1])
            {
                printf("Clicked BUTTON 2\n");
            }
            else
            {
                printf("Released BUTTON 2\n");
            }
        }

        if (gpio_get_level(BUTTON_3) != buttons_state[2])
        {
            buttons_state[2] = !buttons_state[2];
            if (buttons_state[2])
            {
                printf("Clicked BUTTON 3\n");
            }
            else
            {
                printf("Released BUTTON 3\n");
            }
        }
    }

    s_gpio_task_handle = NULL;
    vTaskDelete(NULL);
}

void gpio_task_start_up(void)
{
    xTaskCreate(gpio_task_handler, "gpio_task_handler", 2 * 1024, NULL, 10, &s_gpio_task_handle);
    return;
}

void gpio_task_shut_down(void)
{
    if (s_gpio_task_handle)
    {
        vTaskDelete(s_gpio_task_handle);
        s_gpio_task_handle = NULL;
    }
}
