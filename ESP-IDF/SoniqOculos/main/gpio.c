#include "gpio.h"

#include "i2s.h"
#include "sd.h"
#include "bt.h"
#include "bt_app_av.h"

static int mode = MUSIC;

bool playing_back = false; //! from i2s

static int buttons_map = 0;
static int buttons_command = 0;

static int buttons_pressed(int buttons);

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

static xTaskHandle s_gpio_task_handle = NULL;
static void gpio_task(void *arg);

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

static void releasing_task(void *arg)
{
    delay(RELEASE_DELAY);
    buttons_command = buttons_map;

    s_releasing_task_handle = NULL;
    vTaskDelete(NULL);
}

static void power_off_task(void *arg)
{
    delay(POWER_OFF_HOLD_TIME);
    powering_off = true;

    printf("Powering off...\n");

    //!gpio_task_deinit();
    powering_off = false; //! Deep-sleep

    s_power_off_task_handle = NULL;
    vTaskDelete(NULL);
}

static void volume_task(void *arg)
{
    delay(VOLUME_CHANGE_START_DELAY);

    changed_volume = true;
    for (;;)
    {
        if (buttons_map == B2_MASK)
        {
            printf("Volume up\n");
        }
        if (buttons_map == B3_MASK)
        {
            printf("Volume down\n");
        }

        delay(VOLUME_CHANGE_PERIOD);
    }

    s_volume_task_handle = NULL;
    vTaskDelete(NULL);
}

static void gpio_task(void *arg)
{
    gpio_pad_select_gpio(B1);                   // Set GPIO
    gpio_set_direction(B1, GPIO_MODE_INPUT);    // Set INPUT
    gpio_set_pull_mode(B1, GPIO_PULLDOWN_ONLY); // Set PULLDOWN

    gpio_pad_select_gpio(B2);                   // Set GPIO
    gpio_set_direction(B2, GPIO_MODE_INPUT);    // Set INPUT
    gpio_set_pull_mode(B2, GPIO_PULLDOWN_ONLY); // Set PULLDOWN

    gpio_pad_select_gpio(B3);                   // Set GPIO
    gpio_set_direction(B3, GPIO_MODE_INPUT);    // Set INPUT
    gpio_set_pull_mode(B3, GPIO_PULLDOWN_ONLY); // Set PULLDOWN

    while (gpio_get_level(B1) || gpio_get_level(B2) || gpio_get_level(B3)) // Wait if user pressing any button
        delay(DEBOUNCE);

    for (;;)
    {
        delay(DEBOUNCE);

        if (gpio_get_level(B1) != ((buttons_map & B1_MASK) ? 1 : 0))
        {
            buttons_map ^= B1_MASK;

            if (GPIO_DEBUG)
                (buttons_map & B1_MASK) ? printf("Pressed B1\n") : printf("Released B1\n");

            if (buttons_map & B1_MASK && buttons_map == B1_MASK) // Pressed only button 1
                power_off_task_init();
        }

        if (gpio_get_level(B2) != ((buttons_map & B2_MASK) ? 1 : 0))
        {
            buttons_map ^= B2_MASK;

            if (GPIO_DEBUG)
                (buttons_map & B2_MASK) ? printf("Pressed B2\n") : printf("Released B2\n");

            if (buttons_map & B2_MASK && buttons_map == B2_MASK && mode == MUSIC) // Pressed only button 2 and in music mode
                volume_task_init();
        }

        if (gpio_get_level(B3) != ((buttons_map & B3_MASK) ? 1 : 0))
        {
            buttons_map ^= B3_MASK;

            if (GPIO_DEBUG)
                (buttons_map & B3_MASK) ? printf("Pressed B3\n") : printf("Released B3\n");

            if (buttons_map & B3_MASK && buttons_map == B3_MASK && mode == MUSIC) // Pressed only button 3 and in music mode
                volume_task_init();
        }

        if (buttons_map != buttons_command && buttons_map != B1_MASK)
            power_off_task_deinit(); // Stop powering off when not pressing only button 1

        if (buttons_map != buttons_command && buttons_map != B2_MASK && buttons_map != B3_MASK)
            volume_task_deinit(); // Stop volume change when not pressing only buttons 2 or 3

        if (buttons_pressed(buttons_map) < buttons_pressed(buttons_command)) // When a button is released
            releasing_task_init();                                           // Delay update buttons_map update
        else if (buttons_map != buttons_command)
        {
            releasing_task_deinit();
            buttons_command = buttons_map; // Update buttons state
        }

        if (buttons_map == 0 && buttons_map != buttons_command) // When no button is pressed
        {
            releasing_task_deinit(); // No need

            //? Maybe organize in modes

            switch (buttons_command)
            {
            case B1_MASK: // 001
                switch (mode)
                {
                case MUSIC:
                    !bt_is_music_playing() ? printf("Play\n") : printf("Pause\n");
                    bt_send_cmd(!bt_is_music_playing() ? ESP_AVRC_PT_CMD_PLAY : ESP_AVRC_PT_CMD_PAUSE);
                    delay(COMMAND_DELAY);
                    break;
                case RECORD_PLAYBACK:
                    if (sd_card_init())
                    {
                        printf("Start recording\n");
                        //sd_open_file("testing.txt", "wb");
                    }
                    else
                    {
                        printf("Stop recording\n");
                        sd_card_deinit();
                    }
                    delay(COMMAND_DELAY);
                    break;
                }
                break;

            case B2_MASK: // 010
                if (mode == MUSIC && !changed_volume)
                {
                    printf("Volume up\n");
                }
                break;

            case B3_MASK: // 100
                switch (mode)
                {
                case MUSIC:
                    if (!changed_volume)
                    {
                        printf("Volume down\n");
                    }
                    break;
                case RECORD_PLAYBACK:
                    playing_back = !playing_back;
                    if (playing_back)
                    {
                        printf("Start playback\n");
                        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, ON);
                    }
                    else
                    {
                        printf("Stop playback\n");
                        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, OFF);
                    }
                    delay(COMMAND_DELAY);
                    break;
                }
                break;

            case B1_MASK | B2_MASK: // 011
                if (mode == MUSIC)
                {
                    printf("Next track\n");
                    bt_send_cmd(ESP_AVRC_PT_CMD_FORWARD);
                }
                break;

            case B2_MASK | B3_MASK: // 110
                if (mode == MUSIC)
                {
                    printf("Previous track\n");
                    bt_send_cmd(ESP_AVRC_PT_CMD_BACKWARD);
                }
                break;

            case B1_MASK | B3_MASK: // 101
                if (mode == MUSIC)
                {
                    printf("Change device\n");
                    i2s_change_devices_state();
                    delay(COMMAND_DELAY);
                }
                break;

            case B1_MASK | B2_MASK | B3_MASK: // 111
                mode = !mode;
                switch (mode)
                {
                case MUSIC:
                    printf("Change mode: MUSIC\n");
                    mode = MUSIC;

                    bt_music_init();

                    speakers_init();
                    i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);

                    break;
                case RECORD_PLAYBACK:
                    printf("Change mode: RECORD_PLAYBACK\n");
                    mode = RECORD_PLAYBACK;
                    i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO);
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

    if (!s_gpio_task_handle)
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