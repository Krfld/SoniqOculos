#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define DEBOUNCE 50 // ms

#define BUTTON_1 GPIO_NUM_13
#define BUTTON_2 GPIO_NUM_12
#define BUTTON_3 GPIO_NUM_14

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
    char button_map = 0;
    bool button_1_state = false, button_2_state = false, button_3_state = false;

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

        if (gpio_get_level(BUTTON_1) != button_1_state)
        {
            button_1_state = !button_1_state;
            if (button_1_state)
                printf("Clicked BUTTON 1\n");
            else
                printf("Released BUTTON 1\n");
        }

        if (gpio_get_level(BUTTON_2) != button_2_state)
        {
            button_2_state = !button_2_state;
            if (button_2_state)
                printf("Clicked BUTTON 2\n");
            else
                printf("Released BUTTON 2\n");
        }

        if (gpio_get_level(BUTTON_3) != button_3_state)
        {
            button_3_state = !button_3_state;
            if (button_3_state)
                printf("Clicked BUTTON 3\n");
            else
                printf("Released BUTTON 3\n");
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
