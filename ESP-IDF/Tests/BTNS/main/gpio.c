#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define GPIO_DEBUG false

#define GPIO_STACK_DEPTH 2 * 1024
#define RELEASING_STACK_DEPTH 2 * 1024
#define POWER_OFF_STACK_DEPTH 2 * 1024
#define VOLUME_STACK_DEPTH 2 * 1024

#define DEBOUNCE 50                   // ms
#define RELEASE_DELAY 250             // ms
#define POWER_OFF_HOLD_TIME 2000      // ms
#define VOLUME_CHANGE_START_DELAY 500 // ms
#define VOLUME_CHANGE_PERIOD 500      // ms
#define COMMAND_DELAY 500             // ms

#define B1 GPIO_NUM_13 // Button 1
#define B2 GPIO_NUM_12 // Button 2
#define B3 GPIO_NUM_14 // Button 3
#define B1_MASK 1 << 0
#define B2_MASK 1 << 1
#define B3_MASK 1 << 2

enum MODES
{
    MUSIC,
    RECORD_PLAYBACK
};
enum MODES mode = MUSIC;

enum DEVICES
{
    SPEAKERS_BONE_CONDUCTORS,
    SPEAKERS,
    BONE_CONDUCTORS
};
enum DEVICES device = SPEAKERS_BONE_CONDUCTORS;

static int buttons_map = 0;
static int buttons_command = 0;

bool recording = false;
bool playing_back = false;

void delay(size_t millis);
static int buttons_pressed(int buttons);

static xTaskHandle s_gpio_task_handle = NULL;
static void gpio_task(void *arg);
void gpio_task_init();
void gpio_task_deinit();

static xTaskHandle s_releasing_task_handle = NULL;
static void releasing_task(void *arg);
static void releasing_task_init();
static void releasing_task_deinit();

static bool powering_off = false; // Might not be needed
static xTaskHandle s_power_off_task_handle = NULL;
static void power_off_task(void *arg);
static void power_off_task_init();
static void power_off_task_deinit();

static bool changed_volume = false;
static xTaskHandle s_volume_task_handle = NULL;
static void volume_task(void *arg);
static void volume_task_init();
static void volume_task_deinit();

void delay(size_t millis)
{
    vTaskDelay(millis / portTICK_PERIOD_MS);
}

static int buttons_pressed(int buttons)
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
    gpio_task_init();
}

static void power_off_task(void *arg)
{
    delay(POWER_OFF_HOLD_TIME);
    powering_off = true;

    printf("Powering off...\n");

    gpio_task_deinit();

    s_power_off_task_handle = NULL;
    vTaskDelete(NULL);
}
static void power_off_task_init()
{
    if (GPIO_DEBUG)
        printf("Power off task init\n");

    if (!s_power_off_task_handle)
        xTaskCreate(power_off_task, "power_off_task", POWER_OFF_STACK_DEPTH, NULL, 10, &s_power_off_task_handle);
}
static void power_off_task_deinit()
{
    if (GPIO_DEBUG)
        printf("Power off task deinit\n");

    if (s_power_off_task_handle && !powering_off)
    {
        vTaskDelete(s_power_off_task_handle);
        s_power_off_task_handle = NULL;
    }
}

static void volume_task(void *arg)
{
    delay(VOLUME_CHANGE_START_DELAY);

    changed_volume = true;
    for (;;)
    {
        if (buttons_map == B2_MASK)
            printf("Volume up\n");
        if (buttons_map == B3_MASK)
            printf("Volume down\n");

        delay(VOLUME_CHANGE_PERIOD);
    }

    s_volume_task_handle = NULL;
    vTaskDelete(NULL);
}
static void volume_task_init()
{
    if (GPIO_DEBUG)
        printf("Volume task init\n");

    if (!s_volume_task_handle)
        xTaskCreate(volume_task, "volume_task", VOLUME_STACK_DEPTH, NULL, 10, &s_volume_task_handle);
}
static void volume_task_deinit()
{
    if (GPIO_DEBUG)
        printf("Volume task deinit\n");

    if (s_volume_task_handle)
    {
        vTaskDelete(s_volume_task_handle);
        s_volume_task_handle = NULL;
    }
}

static void releasing_task(void *arg)
{
    delay(RELEASE_DELAY);
    buttons_command = buttons_map;

    s_releasing_task_handle = NULL;
    vTaskDelete(NULL);
}
static void releasing_task_init()
{
    if (GPIO_DEBUG)
        printf("Releasing task init\n");

    if (!s_releasing_task_handle)
        xTaskCreate(releasing_task, "releasing_task", RELEASING_STACK_DEPTH, NULL, 10, &s_releasing_task_handle);
}
static void releasing_task_deinit()
{
    if (GPIO_DEBUG)
        printf("Releasing task deinit\n");

    if (s_releasing_task_handle)
    {
        vTaskDelete(s_releasing_task_handle);
        s_releasing_task_handle = NULL;
    }
}

static void gpio_task(void *arg)
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
        delay(DEBOUNCE);

        if (gpio_get_level(B1) != ((buttons_map & B1_MASK) ? 1 : 0))
        {
            buttons_map ^= B1_MASK;
            if (buttons_map & B1_MASK)
            {
                if (GPIO_DEBUG)
                    printf("Pressed B1\n");

                if (buttons_map == B1_MASK)
                    power_off_task_init();
            }
            else if (GPIO_DEBUG)
                printf("Released B1\n");
        }

        if (gpio_get_level(B2) != ((buttons_map & B2_MASK) ? 1 : 0))
        {
            buttons_map ^= B2_MASK;
            if (buttons_map & B2_MASK)
            {
                if (GPIO_DEBUG)
                    printf("Pressed B2\n");

                if (buttons_map == B2_MASK && mode == MUSIC)
                    volume_task_init();
            }
            else if (GPIO_DEBUG)
                printf("Released B2\n");
        }

        if (gpio_get_level(B3) != ((buttons_map & B3_MASK) ? 1 : 0))
        {
            buttons_map ^= B3_MASK;
            if (buttons_map & B3_MASK)
            {
                if (GPIO_DEBUG)
                    printf("Pressed B3\n");

                if (buttons_map == B3_MASK && mode == MUSIC)
                    volume_task_init();
            }
            else if (GPIO_DEBUG)
                printf("Released B3\n");
        }

        if (buttons_map != buttons_command && buttons_map != B1_MASK)
            power_off_task_deinit();

        if (buttons_map != buttons_command && buttons_map != B2_MASK && buttons_map != B3_MASK)
            volume_task_deinit();

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

            switch (buttons_command)
            {
            case B1_MASK:
                if (mode == MUSIC)
                    printf("Play/Pause\n");
                break;

            case B2_MASK:
                switch (mode)
                {
                case MUSIC:
                    if (!changed_volume)
                        printf("Volume up\n");
                    break;
                case RECORD_PLAYBACK:
                    recording = !recording;
                    if (recording)
                        printf("Start recording\n");
                    else
                        printf("Stop recording\n");
                    delay(COMMAND_DELAY);
                    break;
                }
                break;

            case B3_MASK:
                switch (mode)
                {
                case MUSIC:
                    if (!changed_volume)
                        printf("Volume down\n");
                    break;
                case RECORD_PLAYBACK:
                    playing_back = !playing_back;
                    if (playing_back)
                        printf("Start playback\n");
                    else
                        printf("Stop playback\n");
                    delay(COMMAND_DELAY);
                    break;
                }
                break;

            case B1_MASK | B2_MASK:
                if (mode == MUSIC)
                    printf("Next track\n");
                break;

            case B1_MASK | B3_MASK:
                if (mode == MUSIC)
                    printf("Previous track\n");
                break;

            case B2_MASK | B3_MASK:
                if (mode == MUSIC)
                {
                    if (device++ == BONE_CONDUCTORS)
                        device = SPEAKERS_BONE_CONDUCTORS;

                    printf("Change device: %d\n", device);
                    delay(COMMAND_DELAY);
                }
                break;

            case B1_MASK | B2_MASK | B3_MASK:
                switch (mode)
                {
                case MUSIC:
                    mode = RECORD_PLAYBACK;
                    printf("Change mode: RECORD_PLAYBACK\n");
                    break;
                case RECORD_PLAYBACK:
                    mode = MUSIC;
                    printf("Change mode: MUSIC\n");
                    break;
                }
                delay(COMMAND_DELAY);
                break;
            }

            changed_volume = false;
            buttons_command = 0;
        }
    }

    s_gpio_task_handle = NULL;
    vTaskDelete(NULL);
}
void gpio_task_init()
{
    if (GPIO_DEBUG)
        printf("GPIO task init\n");

    xTaskCreate(gpio_task, "gpio_task", GPIO_STACK_DEPTH, NULL, 10, &s_gpio_task_handle);
}
void gpio_task_deinit()
{
    if (GPIO_DEBUG)
        printf("GPIO task deinit\n");

    releasing_task_deinit();
    power_off_task_deinit();
    volume_task_deinit();

    if (s_gpio_task_handle)
    {
        vTaskDelete(s_gpio_task_handle);
        s_gpio_task_handle = NULL;
    }
}
