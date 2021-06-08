#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"
#include "sd.h"
#include "nvs_tools.h"

#include "dsp.h"

/**
 * TODO
 * debug mode change
 * change sd task to gpio
 * test SD card without filters //?
 * test FIR with SD card
 */

#include "time.h"

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
        bone_conductors_init();
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

    ESP_LOGW(BT_SPP_TAG, "Message recevied %s", msg);

    // Response
    sprintf(msg, "Message received\n");
}
