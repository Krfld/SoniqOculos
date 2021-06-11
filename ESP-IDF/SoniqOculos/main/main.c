#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"
#include "sd.h"
#include "nvs_tools.h"

#include "dsp.h"

/**
 * !Warnings
 * FIR filters might be delaying to much
 */

/**
 * TODO
 * volume
 * test SD card without filters //?
 * sincronize devices
 * test FIR with SD card
 */

void app_main(void)
{
    ESP_LOGW(MAIN_TAG, "Wakeup cause: %d", esp_sleep_get_wakeup_cause()); // 2 - ESP_SLEEP_WAKEUP_EXT0

    nvs_init();
    spi_init();

    bt_init();
    gpio_task_init();

    crossover_init();

    switch (get_mode())
    {
    case MUSIC:
        i2s_init();
        bt_music_init();
        ESP_LOGW(MAIN_TAG, "MUSIC mode ready");
        break;

    case RECORD_PLAYBACK:
        i2s_init();
        i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO); // Set 32 bit I2S
        microphones_init();
        ESP_LOGW(MAIN_TAG, "RECORD_PLAYBACK mode ready");
        break;
    }

    ESP_LOGW(MAIN_TAG, "Free heap: %d", esp_get_free_heap_size());
}

void delay(int millis)
{
    vTaskDelay(pdMS_TO_TICKS(millis));
}

void shutdown()
{
    ESP_LOGE(GPIO_TAG, "Powering off...");

    gpio_task_deinit();

    i2s_turn_devices_off();
    sd_card_deinit();
    bt_music_deinit();

    //? Maybe change to bt_deinit()
    esp_bluedroid_disable();
    esp_bt_controller_disable();

    while (gpio_get_level(B1) || gpio_get_level(B2) || gpio_get_level(B3)) // Wait if user pressing any button
        delay(DEBOUNCE);

    delay(COMMAND_DELAY);

    esp_sleep_enable_ext0_wakeup(B1, HIGH);
    rtc_gpio_pulldown_en(B1);
    esp_deep_sleep_start();
}

void server_welcome_msg(uint32_t handle)
{
    char msg[MSG_BUFFER_SIZE] = "[0]:[100]\n";
    esp_spp_write(handle, MSG_BUFFER_SIZE, (uint8_t *)msg);
}

void handleMsgs(char *msg)
{
    //TODO Implement with flutter

    ESP_LOGW(BT_SPP_TAG, "Message recevied %s", msg);

    // Response
    sprintf(msg, "Message received\n");
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

        ESP_LOGW(GPIO_TAG, "MUSIC mode ready");
        break;

    case RECORD_PLAYBACK:
        //sd_card_deinit(); //! TESTING

        //TODO Debug

        i2s_turn_devices_off();
        bt_music_deinit();

        i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO); // Set 32 bit I2S
        microphones_init();

        ESP_LOGW(GPIO_TAG, "RECORD_PLAYBACK mode ready");
        break;

    default:
        break;
    }
}

void music_toggle_devices()
{
    i2s_change_devices_state();
}

void record_toggle_sd_card()
{
    if (!sd_card_state())
    {
        ESP_LOGI(GPIO_TAG, "Start recording");
        sd_card_init(); // Mount SD card and create file to write
    }
    else
    {
        ESP_LOGI(GPIO_TAG, "Stop recording");
        sd_card_deinit(); // Close file and unmount SD card
    }
}

void record_toggle_bone_conductors()
{
    if (!i2s_get_device_state(BONE_CONDUCTORS_I2S_NUM))
    {
        ESP_LOGI(GPIO_TAG, "Start playback");
        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, ON); // Turning bone conductors on starts playback
    }
    else
    {
        ESP_LOGI(GPIO_TAG, "Stop playback");
        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, OFF); // Turning bone conductors off stops playback
    }
}