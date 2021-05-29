#include "gpio.h"

#include "i2s.h"
#include "sd.h"
#include "bt.h"
#include "bt_app_av.h"

static int mode = MUSIC;

static int buttons_map = 0;
static int buttons_command = 0;

static int buttons_pressed(int buttons);

static xTaskHandle gpio_task_handle = NULL;
static void gpio_task(void *arg);

static xTaskHandle releasing_task_handle = NULL;
static void releasing_task(void *arg);
static void releasing_task_init();
static void releasing_task_deinit();

static QueueHandle_t power_off_queue_handle = NULL;
static xTaskHandle power_off_task_handle = NULL;
static void power_off_task(void *arg);
static void power_off_task_init();
static void power_off_task_deinit();

static bool changed_volume = false;
static xTaskHandle volume_task_handle = NULL;
static void volume_task(void *arg);
static void volume_task_init();
static void volume_task_deinit();

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

    releasing_task_handle = NULL;
    vTaskDelete(NULL);
}

static void power_off_task(void *arg)
{
    bool powering_off = false;
    for (;;)
    {
        if (xQueueGenericReceive(power_off_queue_handle, &powering_off, portMAX_DELAY, false) && powering_off)
        {
            if (!xQueueGenericReceive(power_off_queue_handle, NULL, pdMS_TO_TICKS(POWER_OFF_HOLD_TIME), false)) // if timeout occurs (2 seconds)
            {
                //gpio_task_deinit(); //! Deep-sleep

                /*while (gpio_get_level(B1) || gpio_get_level(B2) || gpio_get_level(B3)) // Wait if user pressing any button
                    delay(DEBOUNCE);*/

                ESP_LOGE(GPIO_TAG, "Powering off...");

                /*esp_sleep_enable_ext0_wakeup(B1, HIGH);
                rtc_gpio_pullup_en(B1); //TODO Test if needed
                esp_deep_sleep_start();*/
            }
            powering_off = false;
        }
    }

    power_off_task_handle = NULL;
    vTaskDelete(NULL);

    /*esp_sleep_enable_ulp_wakeup(); // Set wakeup reason
    rtc_gpio_isolate(B1); // Isolate so it doesn't draw current on the pulldown resistors
    rtc_gpio_isolate(B2); // Isolate so it doesn't draw current on the pulldown resistors
    rtc_gpio_isolate(B3); // Isolate so it doesn't draw current on the pulldown resistors
    esp_deep_sleep_start();*/
}

static void volume_task(void *arg)
{
    delay(VOLUME_CHANGE_START_DELAY);

    changed_volume = true;
    for (;;)
    {
        if (buttons_map == B2_MASK)
        {
            ESP_LOGI(GPIO_TAG, "Volume up");
        }
        if (buttons_map == B3_MASK)
        {
            ESP_LOGI(GPIO_TAG, "Volume down");
        }

        delay(VOLUME_CHANGE_PERIOD);
    }

    volume_task_handle = NULL;
    vTaskDelete(NULL);
}

static void gpio_task(void *arg)
{
    power_off_task_init();

    rtc_gpio_deinit(B1);
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
        if (gpio_get_level(B1) != ((buttons_map & B1_MASK) ? 1 : 0))
        {
            buttons_map ^= B1_MASK;

            if (GPIO_DEBUG)
            {
                if (buttons_map & B1_MASK)
                    ESP_LOGI(GPIO_TAG, "Pressed B1");
                else
                    ESP_LOGI(GPIO_TAG, "Released B1");
            }

            ////if ((buttons_map & B1_MASK) && buttons_map == B1_MASK) // Pressed only button 1
            ////power_off_task_init();
        }

        if (gpio_get_level(B2) != ((buttons_map & B2_MASK) ? 1 : 0))
        {
            buttons_map ^= B2_MASK;

            if (GPIO_DEBUG)
            {
                if (buttons_map & B2_MASK)
                    ESP_LOGI(GPIO_TAG, "Pressed B2");
                else
                    ESP_LOGI(GPIO_TAG, "Released B2");
            }

            if ((buttons_map & B2_MASK) && buttons_map == B2_MASK && mode == MUSIC) // Pressed only button 2 and in music mode
                volume_task_init();
        }

        if (gpio_get_level(B3) != ((buttons_map & B3_MASK) ? 1 : 0))
        {
            buttons_map ^= B3_MASK;

            if (GPIO_DEBUG)
            {
                if (buttons_map & B3_MASK)
                    ESP_LOGI(GPIO_TAG, "Pressed B3");
                else
                    ESP_LOGI(GPIO_TAG, "Released B3");
            }

            if ((buttons_map & B3_MASK) && buttons_map == B3_MASK && mode == MUSIC) // Pressed only button 3 and in music mode
                volume_task_init();
        }

        delay(DEBOUNCE); // Debounce here to make sure tasks are created before trying to delete them

        if (buttons_map != buttons_command && buttons_map != B1_MASK)
            power_off_task_deinit(); // Stop powering off when not pressing only button 1

        if (buttons_map != buttons_command && buttons_map != B2_MASK && buttons_map != B3_MASK)
            volume_task_deinit(); // Stop changing volume when not pressing only buttons 2 or 3

        if (buttons_pressed(buttons_map) < buttons_pressed(buttons_command)) // When a button is released
            releasing_task_init();                                           // Delay buttons state update
        else if (buttons_map != buttons_command)
        {
            releasing_task_deinit();
            buttons_command = buttons_map; // Update buttons state
        }

        if (buttons_map == 0 && buttons_map != buttons_command) // When no button is pressed
        {
            releasing_task_deinit(); // No need

            switch (mode)
            {
            case MUSIC:
                switch (buttons_command)
                {
                case B1_MASK: // 001
                    if (!bt_is_music_playing())
                    {
                        ESP_LOGI(GPIO_TAG, "Play");
                        bt_send_cmd(ESP_AVRC_PT_CMD_PLAY);
                    }
                    else
                    {
                        ESP_LOGI(GPIO_TAG, "Pause");
                        bt_send_cmd(ESP_AVRC_PT_CMD_PAUSE);
                    }
                    delay(COMMAND_DELAY / 2);
                    break;
                case B2_MASK: // 010
                    if (!changed_volume)
                    {
                        ESP_LOGI(GPIO_TAG, "Volume up");

                        //! Testing
                        if (!sd_file_state())
                        {
                            ESP_LOGI(GPIO_TAG, "Start recording");
                            sd_card_init();
                        }
                        else
                        {
                            ESP_LOGI(GPIO_TAG, "Stop recording");
                            sd_card_deinit();
                        }
                    }
                    break;
                case B3_MASK: // 100
                    if (!changed_volume)
                    {
                        ESP_LOGI(GPIO_TAG, "Volume down");
                    }
                    break;

                case B1_MASK | B2_MASK: // 011
                    ESP_LOGI(GPIO_TAG, "Next track");
                    bt_send_cmd(ESP_AVRC_PT_CMD_FORWARD);
                    break;
                case B2_MASK | B3_MASK: // 110
                    ESP_LOGI(GPIO_TAG, "Previous track");
                    bt_send_cmd(ESP_AVRC_PT_CMD_BACKWARD);
                    break;
                case B1_MASK | B3_MASK: // 101
                    ESP_LOGI(GPIO_TAG, "Change devices");
                    i2s_change_devices_state();
                    delay(COMMAND_DELAY);
                    break;

                case B1_MASK | B2_MASK | B3_MASK: // 111
                    ESP_LOGI(GPIO_TAG, "Change to RECORD_PLAYBACK mode");
                    mode = !mode;

                    turn_devices_off();
                    bt_music_deinit();

                    i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO); // Set 32 bit I2S
                    microphones_init();

                    ESP_LOGW(GPIO_TAG, "RECORD_PLAYBACK mode ready");
                    delay(COMMAND_DELAY);
                    break;

                default:
                    break;
                }
                break;

            case RECORD_PLAYBACK:
                switch (buttons_command)
                {
                case B1_MASK: // 001
                    if (!sd_file_state())
                    {
                        ESP_LOGI(GPIO_TAG, "Start recording");
                        sd_card_init();
                    }
                    else
                    {
                        ESP_LOGI(GPIO_TAG, "Stop recording");
                        sd_card_deinit();
                    }
                    delay(COMMAND_DELAY);
                    break;
                case B3_MASK: // 100
                    if (!i2s_get_device_state(BONE_CONDUCTORS_I2S_NUM))
                    {
                        ESP_LOGI(GPIO_TAG, "Start playback");
                        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, ON);
                    }
                    else
                    {
                        ESP_LOGI(GPIO_TAG, "Stop playback");
                        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, OFF);
                    }
                    delay(COMMAND_DELAY);
                    break;

                case B1_MASK | B2_MASK | B3_MASK: // 111
                    ESP_LOGI(GPIO_TAG, "Change to MUSIC mode");
                    mode = !mode;

                    turn_devices_off();
                    sd_card_deinit();

                    i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO); // Set 16 bit I2S
                    speakers_init();
                    bt_music_init();

                    ESP_LOGW(GPIO_TAG, "MUSIC mode ready");
                    delay(COMMAND_DELAY);
                    break;

                default:
                    break;
                }
                break;
            }

            changed_volume = false;
            buttons_command = 0;
        }
    }

    gpio_task_handle = NULL;
    vTaskDelete(NULL);
}

void gpio_task_init()
{
    if (!gpio_task_handle)
        xTaskCreate(gpio_task, "gpio_task", GPIO_STACK_DEPTH, NULL, 10, &gpio_task_handle);
}
void gpio_task_deinit()
{
    if (gpio_task_handle)
    {
        vTaskDelete(gpio_task_handle);
        gpio_task_handle = NULL;
    }
}

static void releasing_task_init()
{
    if (!releasing_task_handle)
        xTaskCreate(releasing_task, "releasing_task", RELEASING_STACK_DEPTH, NULL, 10, &releasing_task_handle);
}
static void releasing_task_deinit()
{
    if (releasing_task_handle)
    {
        vTaskDelete(releasing_task_handle);
        releasing_task_handle = NULL;
    }
}

static void power_off_task_init()
{
    if (!power_off_task_handle)
        if (xTaskCreate(power_off_task, "power_off_task", POWER_OFF_STACK_DEPTH, NULL, 10, &power_off_task_handle) != pdPASS)
            ESP_LOGE(GPIO_TAG, "Error creating power off task");

    power_off_queue_handle = xQueueCreate(1, sizeof(bool));
    if (!power_off_queue_handle)
        ESP_LOGE(GPIO_TAG, "Error creating power off queue");
}
static void power_off_task_deinit()
{
    if (power_off_task_handle)
    {
        vTaskDelete(power_off_task_handle);
        power_off_task_handle = NULL;
    }

    delay(10); // Make sure task is deleted before deleting queue

    if (power_off_queue_handle)
    {
        vQueueDelete(power_off_queue_handle);
        power_off_queue_handle = NULL;
    }
}

static void volume_task_init()
{
    if (!volume_task_handle)
        xTaskCreate(volume_task, "volume_task", VOLUME_STACK_DEPTH, NULL, 10, &volume_task_handle);
}
static void volume_task_deinit()
{
    if (volume_task_handle)
    {
        vTaskDelete(volume_task_handle);
        volume_task_handle = NULL;
    }
}