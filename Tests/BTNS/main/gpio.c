#include <stdio.h>
////#include <string.h>
////#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define DEBOUNCE 100 // ms

#define BUTTON_START GPIO_NUM_13
#define BUTTON_VOLUME_UP GPIO_NUM_12
#define BUTTON_VOLUME_DOWN GPIO_NUM_14

static xTaskHandle s_gpio_task_handle = NULL;

static void gpio_task_handler(void *arg);
void gpio_task_start_up(void);
void gpio_task_shut_down(void);

void app_main(void)
{
    gpio_task_start_up();
}

static void gpio_task_handler(void *arg)
{
    bool button_start_state = false, button_volume_up_state = false, button_volume_down_state = false;

    gpio_pad_select_gpio(BUTTON_START);
    gpio_set_direction(BUTTON_START, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_START, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(BUTTON_VOLUME_UP);
    gpio_set_direction(BUTTON_VOLUME_UP, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_VOLUME_UP, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(BUTTON_VOLUME_DOWN);
    gpio_set_direction(BUTTON_VOLUME_DOWN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_VOLUME_DOWN, GPIO_PULLDOWN_ONLY);

    for (;;)
    {
        vTaskDelay(DEBOUNCE / portTICK_PERIOD_MS);

        if (gpio_get_level(BUTTON_START) != button_start_state)
        {
            button_start_state = !button_start_state;
            if (button_start_state)
                printf("Clicked START\n");
            else
                printf("Released START\n");
        }

        if (gpio_get_level(BUTTON_VOLUME_UP) != button_volume_up_state)
        {
            button_volume_up_state = !button_volume_up_state;
            if (button_volume_up_state)
                printf("Clicked VOLUME UP\n");
            else
                printf("Released VOLUME UP\n");
        }

        if (gpio_get_level(BUTTON_VOLUME_DOWN) != button_volume_down_state)
        {
            button_volume_down_state = !button_volume_down_state;
            if (button_volume_down_state)
                printf("Clicked VOLUME DOWN\n");
            else
                printf("Released VOLUME DOWN\n");
        }
    }
}

void gpio_task_start_up(void)
{
    xTaskCreate(gpio_task_handler, "gpio_task_handler", 1024, NULL, 10, &s_gpio_task_handle);
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
