#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"
#include "sd.h"
#include "nvs_tools.h"

#include "dsp.h"

#define MAIN_TAG "MAIN"

/**
 * !Warnings
 */

/**
 * TODO
 * sincronize devices
 * shelf FIRs to equalize
 */

void app_main(void)
{
    ESP_LOGW(MAIN_TAG, "Wakeup cause: %d", esp_sleep_get_wakeup_cause()); // 2 - ESP_SLEEP_WAKEUP_EXT0

    nvs_init(); //* Flash to store non-volatile data
    spi_init(); //* SPI to comunicate with SD card

    bt_init();        //* Start BT SPP server
    i2s_init();       //* Setup I2S interface
    gpio_task_init(); //* Start task to handle GPIOs

    crossover_init();

    switch (get_mode())
    {
    case MUSIC:
        speakers_init();
        bt_music_init();
        ESP_LOGW(MAIN_TAG, "MUSIC mode ready");
        break;

    case RECORD_PLAYBACK:
        microphones_init();
        ESP_LOGW(MAIN_TAG, "RECORD_PLAYBACK mode ready");
        break;
    }

    ESP_LOGW(MAIN_TAG, "Free heap: %d", esp_get_free_heap_size());

    if (GPIO_DEBUG)
        ESP_LOGW(MAIN_TAG, "GPIO_DEBUG ON");
    if (I2S_DEBUG)
        ESP_LOGW(MAIN_TAG, "I2S_DEBUG ON");
    if (SD_DEBUG)
        ESP_LOGW(MAIN_TAG, "SD_DEBUG ON");
    if (BT_DEBUG)
        ESP_LOGW(MAIN_TAG, "BT_DEBUG ON");
    if (DSP_DEBUG)
        ESP_LOGW(MAIN_TAG, "DSP_DEBUG ON");
}

void delay(int millis)
{
    vTaskDelay(pdMS_TO_TICKS(millis));
}

void shutdown()
{
    ESP_LOGE(MAIN_TAG, "Powering off...");

    gpio_task_deinit();

    i2s_turn_devices_off();
    sd_card_deinit();
    bt_music_deinit();

    esp_bluedroid_disable();
    esp_bt_controller_disable();

    while (gpio_get_level(B1) || gpio_get_level(B2) || gpio_get_level(B3)) // Wait if user pressing any button
        delay(DEBOUNCE);

    delay(COMMAND_DELAY);

    esp_sleep_enable_ext0_wakeup(B1, HIGH);
    rtc_gpio_pulldown_en(B1);
    esp_deep_sleep_start();
}

void handleMsgs(char *msg)
{
    //TODO Implement with flutter

    ESP_LOGW(MAIN_TAG, "Message recevied %s", msg);

    spp_send_msg("Message received\n"); // Send response
}

void change_to_mode(int mode)
{
    switch (mode)
    {
    case MUSIC:
        i2s_turn_devices_off();
        sd_card_deinit();

        i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO); // Set 16 bit I2S
        speakers_init();
        bt_music_init();

        ESP_LOGW(MAIN_TAG, "MUSIC mode ready");
        break;

    case RECORD_PLAYBACK:
        //sd_card_deinit(); //! TESTING

        //TODO Debug

        i2s_turn_devices_off();
        bt_music_deinit();

        i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO); // Set 32 bit I2S
        microphones_init();

        ESP_LOGW(MAIN_TAG, "RECORD_PLAYBACK mode ready");
        break;

    default:
        break;
    }
}

void music_toggle_devices()
{
    //TODO Change to specific device
}

void record_toggle_sd_card()
{
    if (!sd_card_state())
    {
        ESP_LOGI(MAIN_TAG, "Start recording");
        sd_card_init(); // Mount SD card and create file to write
    }
    else
    {
        ESP_LOGI(MAIN_TAG, "Stop recording");
        sd_card_deinit(); // Close file and unmount SD card
    }
}

void record_toggle_bone_conductors()
{
    if (!i2s_get_device_state(BONE_CONDUCTORS_I2S_NUM))
    {
        ESP_LOGI(MAIN_TAG, "Start playback");
        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, ON); // Turning bone conductors on starts playback
    }
    else
    {
        ESP_LOGI(MAIN_TAG, "Stop playback");
        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, OFF); // Turning bone conductors off stops playback
    }
}