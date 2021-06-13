#include "gpio.h"

#include "bt.h"
#include "sd.h"

const bool _true_ = true;
const bool _false_ = false;

RTC_DATA_ATTR static int mode = MUSIC; //* Keep value while in deep-sleep
int get_mode()
{
    return mode;
}

RTC_DATA_ATTR static int volume = 50; //* Keep value while in deep-sleep
int get_volume()
{
    return volume;
}
void volume_up()
{
    if (volume + VOLUME_INCREMENTS > MAX_VOLUME)
        volume = MAX_VOLUME;
    else
        volume += VOLUME_INCREMENTS;
}
void volume_down()
{
    if (volume - VOLUME_INCREMENTS < 0)
        volume = 0;
    else
        volume -= VOLUME_INCREMENTS;
}

static bool sd_det_state = false;
bool get_sd_det_state()
{
    return sd_det_state;
}

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
////static void releasing_task_deinit();

static QueueHandle_t power_off_queue_handle = NULL;
static xTaskHandle power_off_task_handle = NULL;
static void power_off_task(void *arg);
static void power_off_task_init();
////static void power_off_task_deinit();

static bool changed_volume = false;
static QueueHandle_t volume_queue_handle = NULL;
static xTaskHandle volume_task_handle = NULL;
static void volume_task(void *arg);
static void volume_task_init();
////static void volume_task_deinit();

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
}

static void power_off_task(void *arg)
{
    bool powering_off;
    for (;;)
        if (xQueueReceive(power_off_queue_handle, &powering_off, portMAX_DELAY) && powering_off)           // Wait for true value
            if (!xQueueReceive(power_off_queue_handle, &powering_off, pdMS_TO_TICKS(POWER_OFF_HOLD_TIME))) // Timeout occurs (2 seconds)
                shutdown();
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
                    volume_up();
                }
                if (buttons_map == B3_MASK)
                {
                    ESP_LOGI(GPIO_TAG, "Volume down");
                    volume_down();
                }
            }
        }
    }
}

static void gpio_task(void *arg)
{
    gpio_pad_select_gpio(SPEAKERS_SD_PIN);                 // Set GPIO
    gpio_set_direction(SPEAKERS_SD_PIN, GPIO_MODE_OUTPUT); // Set OUTPUT

    gpio_pad_select_gpio(BONE_CONDUCTORS_SD_PIN);                 // Set GPIO
    gpio_set_direction(BONE_CONDUCTORS_SD_PIN, GPIO_MODE_OUTPUT); // Set OUTPUT

    gpio_pad_select_gpio(SD_DET_PIN);                // Set GPIO
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT); // Set INPUT

    releasing_task_init();
    power_off_task_init();
    volume_task_init();

    rtc_gpio_deinit(B1);                        // Deinit RTC_GPIO
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
        delay(DEBOUNCE); // Debounce delay

        if (gpio_get_level(SD_DET_PIN) != sd_det_state)
        {
            sd_det_state = !sd_det_state;

            if (sd_det_state)
                ESP_LOGW(SD_CARD_TAG, "Card inserted");
            else // SD removed
            {
                ESP_LOGW(SD_CARD_TAG, "Card removed");

                if (sd_card_state()) // Check if card was removed unexpectedly
                {
                    ESP_LOGE(SD_CARD_TAG, "SD fault");
                    nvs_write(nvs_read(FILE_NAME) - 1, FILE_NAME);
                }

                sd_card_deinit();
            }
        }

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
                        volume_up();
                        //! TESTING
                        record_toggle_sd_card();
                        delay(COMMAND_DELAY);
                    }
                    break;
                case B3_MASK: // 100
                    if (!changed_volume)
                    {
                        ESP_LOGI(GPIO_TAG, "Volume down");
                        volume_down();
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
                    change_to_mode(mode);
                    delay(COMMAND_DELAY);
                    break;

                default:
                    break;
                }
                break;

            case RECORD_PLAYBACK:
                switch (buttons_command)
                {
                case B1_MASK:
                case B3_MASK:
                case B1_MASK | B2_MASK:
                case B3_MASK | B2_MASK:
                case B1_MASK | B3_MASK:            // 101
                    if (buttons_command & B1_MASK) // 001
                        record_toggle_sd_card();
                    if (buttons_command & B3_MASK) // 100
                        record_toggle_bone_conductors();
                    delay(COMMAND_DELAY);
                    break;

                case B1_MASK | B2_MASK | B3_MASK: // 111
                    ESP_LOGW(GPIO_TAG, "Change to MUSIC mode");
                    mode = !mode;
                    change_to_mode(mode);
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
    releasing_queue_handle = xQueueCreate(1, sizeof(bool));
    if (!releasing_queue_handle)
        ESP_LOGE(GPIO_TAG, "Error creating RELEASING queue");

    if (!releasing_task_handle)
        if (!xTaskCreate(releasing_task, "releasing_task", RELEASING_STACK_DEPTH, NULL, 10, &releasing_task_handle))
            ESP_LOGE(GPIO_TAG, "Error creating RELEASING task");
}
/*static void releasing_task_deinit()
{
    if (releasing_task_handle)
    {
        vTaskDelete(releasing_task_handle);
        releasing_task_handle = NULL;
    }
    delay(100); // Make sure task is deleted before deleting queue
    if (releasing_queue_handle)
    {
        vQueueDelete(releasing_queue_handle);
        releasing_queue_handle = NULL;
    }
}*/

static void power_off_task_init()
{
    power_off_queue_handle = xQueueCreate(1, sizeof(bool));
    if (!power_off_queue_handle)
        ESP_LOGE(GPIO_TAG, "Error creating POWER OFF queue");

    if (!power_off_task_handle)
        if (!xTaskCreate(power_off_task, "power_off_task", POWER_OFF_STACK_DEPTH, NULL, 10, &power_off_task_handle))
            ESP_LOGE(GPIO_TAG, "Error creating POWER OFF task");
}
/*static void power_off_task_deinit()
{
    if (power_off_task_handle)
    {
        vTaskDelete(power_off_task_handle);
        power_off_task_handle = NULL;
    }
    delay(100); // Make sure task is deleted before deleting queue
    if (power_off_queue_handle)
    {
        vQueueDelete(power_off_queue_handle);
        power_off_queue_handle = NULL;
    }
}*/

static void volume_task_init()
{
    volume_queue_handle = xQueueCreate(1, sizeof(bool));
    if (!volume_queue_handle)
        ESP_LOGE(GPIO_TAG, "Error creating VOLUME queue");

    if (!volume_task_handle)
        xTaskCreate(volume_task, "volume_task", VOLUME_STACK_DEPTH, NULL, 10, &volume_task_handle);
}
/*static void volume_task_deinit()
{
    if (volume_task_handle)
    {
        vTaskDelete(volume_task_handle);
        volume_task_handle = NULL;
    }
    delay(100); // Make sure task is deleted before deleting queue
    if (volume_queue_handle)
    {
        vQueueDelete(volume_queue_handle);
        volume_queue_handle = NULL;
    }
}*/