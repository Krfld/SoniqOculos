#include "gpio.h"

#include "i2s.h"
#include "sd.h"

bool recording = false;    // from sd
bool playing_back = false; // from i2s

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

void wait_for_sd_card()
{
    gpio_pad_select_gpio(SD_DET_PIN);
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT);

    do // Make sure the card is inserted
    {
        while (!gpio_get_level(SD_DET_PIN))
            ;
        delay(SD_CARD_DET_DELAY);
    } while (!gpio_get_level(SD_DET_PIN));
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

void i2s_pins_reset(int ws_pin, int bck_pin, int data_pin)
{
    gpio_pad_select_gpio(ws_pin);                 // Set GPIO
    gpio_set_direction(ws_pin, GPIO_MODE_OUTPUT); // Set OUTPUT
    gpio_set_level(ws_pin, LOW);                  // Set LOW

    gpio_pad_select_gpio(bck_pin);                 // Set GPIO
    gpio_set_direction(bck_pin, GPIO_MODE_OUTPUT); // Set OUTPUT
    gpio_set_level(bck_pin, LOW);

    gpio_pad_select_gpio(data_pin);                 // Set GPIO
    gpio_set_direction(data_pin, GPIO_MODE_OUTPUT); // Set OUTPUT
    gpio_set_level(data_pin, LOW);                  // Set LOW
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
    powering_off = false; //!

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
            printf("Volume up\n");
        if (buttons_map == B3_MASK)
            printf("Volume down\n");

        delay(VOLUME_CHANGE_PERIOD);
    }

    s_volume_task_handle = NULL;
    vTaskDelete(NULL);
}

static void gpio_task(void *arg)
{
    bool sd_det_state = false;

    gpio_pad_select_gpio(B1);                   // Set GPIO
    gpio_set_direction(B1, GPIO_MODE_INPUT);    // Set INPUT
    gpio_set_pull_mode(B1, GPIO_PULLDOWN_ONLY); // Set PULLDOWN

    gpio_pad_select_gpio(B2);                   // Set GPIO
    gpio_set_direction(B2, GPIO_MODE_INPUT);    // Set INPUT
    gpio_set_pull_mode(B2, GPIO_PULLDOWN_ONLY); // Set PULLDOWN

    gpio_pad_select_gpio(B3);                   // Set GPIO
    gpio_set_direction(B3, GPIO_MODE_INPUT);    // Set INPUT
    gpio_set_pull_mode(B3, GPIO_PULLDOWN_ONLY); // Set PULLDOWN

    gpio_pad_select_gpio(SD_DET_PIN);                // Set GPIO
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT); // Set INPUT

    for (;;)
    {
        delay(DEBOUNCE);

        if (gpio_get_level(B1) != ((buttons_map & B1_MASK) ? 1 : 0))
        {
            buttons_map ^= B1_MASK;
            if (buttons_map & B1_MASK) // Pressed button 1
            {
                if (GPIO_DEBUG)
                    printf("Pressed B1\n");

                if (buttons_map == B1_MASK)
                    power_off_task_init();
            }
            else if (GPIO_DEBUG) // Released button 1
                printf("Released B1\n");
        }

        if (gpio_get_level(B2) != ((buttons_map & B2_MASK) ? 1 : 0))
        {
            buttons_map ^= B2_MASK;
            if (buttons_map & B2_MASK) // Pressed button 2
            {
                if (GPIO_DEBUG)
                    printf("Pressed B2\n");

                if (buttons_map == B2_MASK && get_mode() == MUSIC)
                    volume_task_init();
            }
            else if (GPIO_DEBUG) // Released button 2
                printf("Released B2\n");
        }

        if (gpio_get_level(B3) != ((buttons_map & B3_MASK) ? 1 : 0))
        {
            buttons_map ^= B3_MASK;
            if (buttons_map & B3_MASK) // Pressed button 3
            {
                if (GPIO_DEBUG)
                    printf("Pressed B3\n");

                if (buttons_map == B3_MASK && get_mode() == MUSIC)
                    volume_task_init();
            }
            else if (GPIO_DEBUG) // Released button 3
                printf("Released B3\n");
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

            switch (buttons_command)
            {
            case B1_MASK: // 001
                if (get_mode() == MUSIC)
                    printf("Play/Pause\n");
                break;

            case B2_MASK: // 010
                sd_init();
                switch (get_mode())
                {
                case MUSIC:
                    if (!changed_volume)
                        printf("Volume up\n");
                    break;
                case RECORD_PLAYBACK:
                    //TODO Check SD card
                    recording = !recording;
                    if (recording)
                        printf("Start recording\n");
                    else
                        printf("Stop recording\n");
                    delay(COMMAND_DELAY);
                    break;
                }
                break;

            case B3_MASK: // 100
                sd_deinit();
                switch (get_mode())
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

            case B1_MASK | B2_MASK: // 011
                if (get_mode() == MUSIC)
                    printf("Next track\n");
                break;

            case B1_MASK | B3_MASK: // 101
                if (get_mode() == MUSIC)
                    printf("Previous track\n");
                break;

            case B2_MASK | B3_MASK: // 110
                if (get_mode() == MUSIC)
                {
                    printf("Change device\n");
                    /*if (device++ == BONE_CONDUCTORS)
                        device = SPEAKERS_BONE_CONDUCTORS;

                    printf("Change device: %d\n", device);*/
                    delay(COMMAND_DELAY);
                }
                break;

            case B1_MASK | B2_MASK | B3_MASK: // 111
                switch (get_mode())
                {
                case MUSIC:
                    set_mode(RECORD_PLAYBACK);
                    printf("Change mode: RECORD_PLAYBACK\n");
                    break;
                case RECORD_PLAYBACK:
                    set_mode(MUSIC);
                    printf("Change mode: MUSIC\n");
                    break;
                }
                delay(COMMAND_DELAY);
                break;
            }

            changed_volume = false;
            buttons_command = 0;
        }

        /*if (gpio_get_level(SD_DET_PIN) != sd_det_state)
        {
            sd_det_state = !sd_det_state;
            if (sd_det_state)
            {
                printf("Inserting card...\n");
                delay(SD_CARD_DET_DELAY);
                if (gpio_get_level(SD_DET_PIN))
                {
                    printf("Card inserted\n");
                    sd_init();
                    sd_open_file("samples.txt", "wb");
                }
            }
            else
            {
                printf("Removing card...\n");
                sd_deinit();
                printf("Card removed\n");
            }
        }*/
    }
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