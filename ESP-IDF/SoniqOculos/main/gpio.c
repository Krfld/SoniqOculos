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

static bool releasing = false;
static QueueHandle_t releasing_queue_handle = NULL;
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
static QueueHandle_t volume_queue_handle = NULL;
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
    for (;;)
    {
        if (xQueueReceive(releasing_queue_handle, &releasing, portMAX_DELAY) && releasing)                       // Wait for true value
            while (xQueueReceive(releasing_queue_handle, &releasing, pdMS_TO_TICKS(RELEASE_DELAY)) && releasing) // Leave while loop after release delay or if false received
                ;
        buttons_command = buttons_map;
    }
    ////releasing_task_handle = NULL;
    ////vTaskDelete(NULL);
}

static void power_off_task(void *arg)
{
    bool powering_off;
    for (;;)
    {
        if (xQueueReceive(power_off_queue_handle, &powering_off, portMAX_DELAY) && powering_off)           // Wait for true value
            if (!xQueueReceive(power_off_queue_handle, &powering_off, pdMS_TO_TICKS(POWER_OFF_HOLD_TIME))) // Enter deep-sleep mode when timeout occurs (2 seconds)
            {
                //gpio_task_deinit(); //! Deep-sleep

                /*while (gpio_get_level(B1) || gpio_get_level(B2) || gpio_get_level(B3)) // Wait if user pressing any button
                    delay(DEBOUNCE);*/

                ESP_LOGE(GPIO_TAG, "Powering off...");

                /*esp_sleep_enable_ext0_wakeup(B1, HIGH);
                    rtc_gpio_pullup_en(B1); //TODO Test if needed
                    esp_deep_sleep_start();*/
            }
    }
    ////power_off_task_handle = NULL;
    /////vTaskDelete(NULL);

    /*esp_sleep_enable_ulp_wakeup(); // Set wakeup reason
    rtc_gpio_isolate(B1); // Isolate so it doesn't draw current on the pulldown resistors
    rtc_gpio_isolate(B2); // Isolate so it doesn't draw current on the pulldown resistors
    rtc_gpio_isolate(B3); // Isolate so it doesn't draw current on the pulldown resistors
    esp_deep_sleep_start();*/
}

static void volume_task(void *arg)
{
    bool changing_volume;
    for (;;)
    {
        if (xQueueReceive(volume_queue_handle, &changing_volume, portMAX_DELAY) && changing_volume)
        {
            while (!xQueuePeek(volume_queue_handle, &changing_volume, pdMS_TO_TICKS(VOLUME_CHANGE_PERIOD)))
            {
                changed_volume = true;
                if (buttons_map == B2_MASK)
                {
                    ESP_LOGI(GPIO_TAG, "Volume up");
                }
                if (buttons_map == B3_MASK)
                {
                    ESP_LOGI(GPIO_TAG, "Volume down");
                }
            }
        }
    }
    ////volume_task_handle = NULL;
    ////vTaskDelete(NULL);
}

static void gpio_task(void *arg)
{
    releasing_task_init();
    power_off_task_init();
    volume_task_init();

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

    const bool _true_ = true;
    const bool _false_ = false;
    for (;;)
    {
        delay(DEBOUNCE); // Debounce delay

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

            if ((buttons_map & B1_MASK) && buttons_map == B1_MASK) // Pressed only button 1
                xQueueOverwrite(power_off_queue_handle, &_true_);  // Start 2 second wait for power off
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
                xQueueOverwrite(volume_queue_handle, &_true_);
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
                xQueueOverwrite(volume_queue_handle, &_true_);
        }

        if (buttons_map != buttons_command && buttons_map != B1_MASK)
            xQueueOverwrite(power_off_queue_handle, &_false_); // Stop powering off when not pressing only button 1

        if (buttons_map != buttons_command && buttons_map != B2_MASK && buttons_map != B3_MASK)
            xQueueOverwrite(volume_queue_handle, &_false_); // Stop changing volume when not pressing only buttons 2 or 3

        if (buttons_map != buttons_command)
        {
            if (buttons_pressed(buttons_map) < buttons_pressed(buttons_command)) // When a button is released
            {
                if (!releasing)                                       // Check if not releasing already
                    xQueueOverwrite(releasing_queue_handle, &_true_); // Delay buttons state update
            }
            else
                xQueueOverwrite(releasing_queue_handle, &_false_); // Update buttons state immediately
        }

        if (buttons_map == 0 && buttons_map != buttons_command) // When no button is pressed
        {
            switch (mode)
            {
            case MUSIC:
                switch (buttons_command)
                {
                case B1_MASK: // 001
                    if (!bt_is_music_playing())
                    {
                        ESP_LOGI(GPIO_TAG, "Play");
                        bt_send_avrc_cmd(ESP_AVRC_PT_CMD_PLAY); // Send play command if music paused
                    }
                    else
                    {
                        ESP_LOGI(GPIO_TAG, "Pause");
                        bt_send_avrc_cmd(ESP_AVRC_PT_CMD_PAUSE); // Send pause command if music playing
                    }
                    delay(COMMAND_DELAY / 2);
                    break;
                case B2_MASK: // 010
                    if (!changed_volume)
                    {
                        ESP_LOGI(GPIO_TAG, "Volume up");
                        if (!sd_file_state())
                        {
                            ESP_LOGI(GPIO_TAG, "Start recording");
                            sd_card_init(); // Mount SD card and create file to write
                        }
                        else
                        {
                            ESP_LOGI(GPIO_TAG, "Stop recording");
                            sd_card_deinit(); // Close file and unmount SD card
                        }
                        delay(COMMAND_DELAY);
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
                    bt_send_avrc_cmd(ESP_AVRC_PT_CMD_FORWARD); // Send next track command
                    break;
                case B2_MASK | B3_MASK: // 110
                    ESP_LOGI(GPIO_TAG, "Previous track");
                    bt_send_avrc_cmd(ESP_AVRC_PT_CMD_BACKWARD); // Send previous track command
                    break;
                case B1_MASK | B3_MASK: // 101
                    ESP_LOGI(GPIO_TAG, "Change devices");
                    i2s_change_devices_state();
                    delay(COMMAND_DELAY);
                    break;

                case B1_MASK | B2_MASK | B3_MASK: // 111
                    ESP_LOGW(GPIO_TAG, "Change to RECORD_PLAYBACK mode");
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
                        sd_card_init(); // Mount SD card and create file to write
                    }
                    else
                    {
                        ESP_LOGI(GPIO_TAG, "Stop recording");
                        sd_card_deinit(); // Close file and unmount SD card
                    }
                    delay(COMMAND_DELAY);
                    break;
                case B3_MASK: // 100
                    if (!i2s_get_device_state(BONE_CONDUCTORS_I2S_NUM))
                    {
                        ESP_LOGI(GPIO_TAG, "Start playback");
                        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, ON); // Turn bone conductors on and start playback
                    }
                    else
                    {
                        ESP_LOGI(GPIO_TAG, "Stop playback");
                        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, OFF); // Stop playback and turn bone conductors off
                    }
                    delay(COMMAND_DELAY);
                    break;

                case B1_MASK | B2_MASK | B3_MASK: // 111
                    ESP_LOGW(GPIO_TAG, "Change to MUSIC mode");
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
    ////gpio_task_handle = NULL;
    ////vTaskDelete(NULL);
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

    releasing_task_deinit();
    power_off_task_deinit();
    volume_task_deinit();
}

static void releasing_task_init()
{
    releasing_queue_handle = xQueueCreate(1, sizeof(bool));
    if (!releasing_queue_handle)
        ESP_LOGE(GPIO_TAG, "Error creating RELEASING queue");

    if (!releasing_task_handle)
        if (!xTaskCreate(releasing_task, "releasing_task", RELEASING_STACK_DEPTH, NULL, 10, &releasing_task_handle))
            ESP_LOGE(GPIO_TAG, "Error creating RELEASING task");
}
static void releasing_task_deinit()
{
    if (releasing_task_handle)
    {
        vTaskDelete(releasing_task_handle);
        releasing_task_handle = NULL;
    }
    delay(10); // Make sure task is deleted before deleting queue
    if (releasing_queue_handle)
    {
        vQueueDelete(releasing_queue_handle);
        releasing_queue_handle = NULL;
    }
}

static void power_off_task_init()
{
    power_off_queue_handle = xQueueCreate(1, sizeof(bool));
    if (!power_off_queue_handle)
        ESP_LOGE(GPIO_TAG, "Error creating POWER OFF queue");

    if (!power_off_task_handle)
        if (!xTaskCreate(power_off_task, "power_off_task", POWER_OFF_STACK_DEPTH, NULL, 10, &power_off_task_handle))
            ESP_LOGE(GPIO_TAG, "Error creating POWER OFF task");
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
    volume_queue_handle = xQueueCreate(1, sizeof(bool));
    if (!volume_queue_handle)
        ESP_LOGE(GPIO_TAG, "Error creating VOLUME queue");

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
    delay(10); // Make sure task is deleted before deleting queue
    if (volume_queue_handle)
    {
        vQueueDelete(volume_queue_handle);
        volume_queue_handle = NULL;
    }
}