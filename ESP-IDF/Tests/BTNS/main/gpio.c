#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define POWER_OFF_HOLD_TIME 2000      // ms
#define VOLUME_CHANGE_START_DELAY 500 // ms
#define VOLUME_CHANGE_PERIOD 500      // ms
#define RELEASE_DELAY 500             // ms
#define DEBOUNCE 50                   // ms

#define B1 GPIO_NUM_13 // Button 1
#define B2 GPIO_NUM_12 // Button 2
#define B3 GPIO_NUM_14 // Button 3
#define B1_MASK 1 << 0
#define B2_MASK 1 << 1
#define B3_MASK 1 << 2

static int buttons_map = 0;
static int buttons_command = 0;

static bool powering_off = false; // Might not be needed
static bool changing_volume = false;

static xTaskHandle s_power_off_task_handle = NULL;
static void power_off_task(void *arg);
static void power_off_task_init();
static void power_off_task_deinit();

static xTaskHandle s_volume_change_task_handle = NULL;
static void volume_change_task(void *arg);
static void volume_change_task_init();
static void volume_change_task_deinit();

static xTaskHandle s_gpio_task_handle = NULL;
static void gpio_task_handler(void *arg);
static void gpio_task_deinit();

static xTaskHandle s_releasing_task_handle = NULL;
static void releasing_task_handler(void *arg);
static void releasing_task_init();
static void releasing_task_deinit();

void delay(size_t millis)
{
    vTaskDelay(millis / portTICK_PERIOD_MS);
}

int buttons_pressed(int buttons)
{
    int buttons_pressed = 0;

    if (buttons & B1_MASK)
        buttons_pressed++;

    if (buttons & B2_MASK)
        buttons_pressed++;

    if (buttons & B3_MASK)
        buttons_pressed++;

    return buttons_pressed;
}

void app_main(void)
{
    xTaskCreate(gpio_task_handler, "gpio_task_handler", 2 * 1024, NULL, 10, &s_gpio_task_handle);
}

static void power_off_task(void *arg)
{
    delay(POWER_OFF_HOLD_TIME);

    powering_off = true;
    printf("Powering off...\n");

    //gpio_task_deinit();

    s_power_off_task_handle = NULL;
    vTaskDelete(NULL);
}
static void power_off_task_init()
{
    if (!s_power_off_task_handle)
        xTaskCreate(power_off_task, "power_off_task", 2 * 1024, NULL, 10, &s_power_off_task_handle);
}
static void power_off_task_deinit()
{
    if (s_power_off_task_handle)
    {
        vTaskDelete(s_power_off_task_handle);
        s_power_off_task_handle = NULL;
    }
}

static void volume_change_task(void *arg)
{
    delay(VOLUME_CHANGE_START_DELAY);

    changing_volume = true;
    for (;;)
    {
        if (buttons_map == B2_MASK)
            printf("Volume up\n");
        if (buttons_map == B3_MASK)
            printf("Volume down\n");

        delay(VOLUME_CHANGE_PERIOD);
    }

    s_volume_change_task_handle = NULL;
    vTaskDelete(NULL);
}
static void volume_change_task_init()
{
    if (!s_volume_change_task_handle)
        xTaskCreate(volume_change_task, "volume_change_task", 2 * 1024, NULL, 10, &s_volume_change_task_handle);
}
static void volume_change_task_deinit()
{
    if (s_volume_change_task_handle)
    {
        changing_volume = false;
        vTaskDelete(s_volume_change_task_handle);
        s_volume_change_task_handle = NULL;
    }
}

static void releasing_task_handler(void *arg)
{
    delay(RELEASE_DELAY);
    buttons_command = buttons_map;

    s_releasing_task_handle = NULL;
    vTaskDelete(NULL);
}
static void releasing_task_init()
{
    if (!s_releasing_task_handle)
        xTaskCreate(releasing_task_handler, "releasing_task_handler", 2 * 1024, NULL, 10, &s_releasing_task_handle);
}
static void releasing_task_deinit()
{
    if (s_releasing_task_handle)
    {
        vTaskDelete(s_releasing_task_handle);
        s_releasing_task_handle = NULL;
    }
}

static void gpio_task_handler(void *arg)
{

    gpio_pad_select_gpio(B1);
    gpio_set_direction(B1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(B1, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(B2);
    gpio_set_direction(B2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(B2, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(B3);
    gpio_set_direction(B3, GPIO_MODE_INPUT);
    gpio_set_pull_mode(B3, GPIO_PULLDOWN_ONLY);

    for (;;)
    {
        vTaskDelay(DEBOUNCE / portTICK_PERIOD_MS);

        if (gpio_get_level(B1) != ((buttons_map & B1_MASK) ? 1 : 0))
        {
            buttons_map ^= B1_MASK;
            if (buttons_map & B1_MASK)
            {
                printf("Pressed B1\n");

                /*power_off_task_init();*/
            }
            else
            {
                printf("Released B1\n");

                /*power_off_task_deinit();

                if (!powering_off)
                    printf("Play/Pause\n");*/
            }
        }

        if (gpio_get_level(B2) != ((buttons_map & B2_MASK) ? 1 : 0))
        {
            buttons_map ^= B2_MASK;
            if (buttons_map & B2_MASK)
            {
                printf("Pressed B2\n");

                /*power_off_task_deinit();

                volume_change_task_init();*/
            }
            else
            {
                printf("Released B2\n");

                /*if (!changing_volume)
                    printf("Volume up\n");

                volume_change_task_deinit();*/
            }
        }

        if (gpio_get_level(B3) != ((buttons_map & B3_MASK) ? 1 : 0))
        {
            buttons_map ^= B3_MASK;
            if (buttons_map & B3_MASK)
            {
                printf("Pressed B3\n");

                /*power_off_task_deinit();

                volume_change_task_init();*/
            }
            else
            {
                printf("Released B3\n");

                /*if (!changing_volume)
                    printf("Volume down\n");

                volume_change_task_deinit();*/
            }
        }

        if (buttons_pressed(buttons_map) < buttons_pressed(buttons_command)) // When a button is released
            releasing_task_init();
        else if (buttons_map != buttons_command)
        {
            releasing_task_deinit();
            buttons_command = buttons_map; // Update buttons state
        }

        if (buttons_map == 0 && buttons_map != buttons_command) // When no button is pressed
        {
            releasing_task_deinit(); // No need

            printf("Execute command: %d\n\n", buttons_command);
            //TODO Execute command

            buttons_command = 0;
        }
    }

    s_gpio_task_handle = NULL;
    vTaskDelete(NULL);
}
void gpio_task_deinit()
{
    if (s_gpio_task_handle)
    {
        vTaskDelete(s_gpio_task_handle);
        s_gpio_task_handle = NULL;
    }
}
