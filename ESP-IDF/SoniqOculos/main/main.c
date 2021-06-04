#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"
#include "sd.h"

#include "dsp.h"

//TODO flash memory

void app_main(void)
{
    ESP_LOGW(MAIN_TAG, "Wakeup cause: %d", esp_sleep_get_wakeup_cause()); // 2 - ESP_SLEEP_WAKEUP_EXT0

    ESP_LOGW(MAIN_TAG, "Setup init");

    bt_init();
    spi_init();
    sd_det_task_init();
    gpio_task_init();

    crossover_init();

    switch (get_mode())
    {
    case MUSIC:
        i2s_init();
        bt_music_init();
        break;

    case RECORD_PLAYBACK:
        bone_conductors_init();
        i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO); // Set 32 bit I2S
        microphones_init();
        break;
    }

    ESP_LOGW(MAIN_TAG, "Setup ready");
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
    bt_music_deinit();
    sd_card_deinit();

    //esp_spp_deinit();
    esp_bluedroid_disable();
    //esp_bluedroid_deinit();
    esp_bt_controller_disable();

    while (gpio_get_level(B1) || gpio_get_level(B2) || gpio_get_level(B3)) // Wait if user pressing any button
        delay(DEBOUNCE);

    delay(COMMAND_DELAY);

    esp_sleep_enable_ext0_wakeup(B1, HIGH);
    rtc_gpio_pulldown_en(B1);
    //rtc_gpio_isolate(B1); //TODO Test if needed
    esp_deep_sleep_start();
}

void handleMsgs(char *msg)
{
    //TODO Implement with flutter

    ESP_LOGW(BT_SPP_TAG, "Message recevied %s", msg);

    // Response
    sprintf(msg, "Message received\n");
}
