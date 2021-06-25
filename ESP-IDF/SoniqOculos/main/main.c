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
 * write .wav
 * shelf FIRs to equalize
 */

RTC_DATA_ATTR static int mode = MUSIC; //* Keep value while in deep-sleep
int get_mode()
{
    return mode;
}

void app_main(void)
{
    ESP_LOGW(MAIN_TAG, "Wakeup cause: %d", esp_sleep_get_wakeup_cause()); // 2 - ESP_SLEEP_WAKEUP_EXT0

    nvs_init(); //* Flash to store non-volatile data
    spi_init(); //* SPI to comunicate with SD card

    bt_init();        //* Start BT SPP server
    dsp_init();       //* Alocate DSP variables
    i2s_init();       //* Setup I2S interface
    gpio_task_init(); //* Start task to handle GPIOs

    switch (mode)
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
    ESP_LOGW(MAIN_TAG, "Message recevied: '%s'", msg); //TODO Check if "msg\n" or "msg"

    if (spp_get_sending_state())
    {
        if (strcmp(msg, SPP_OK) == 0)
            spp_set_sending_state(false); //* Stop sending if got OK or FAIL
        else
            spp_send_msg(SPP_FAIL); //* Error if msg received while sending
        return;
    }

    char *ptr;
    switch (*msg++)
    {
    case 'm': //* Mode
        change_to_mode(strtol(msg, &ptr, 0));
        break;

    case 'v': //* Volume
        set_volume(strtol(msg, &ptr, 0));
        break;

    case 'd': //* Devices
        i2s_change_to_devices(strtol(msg, &ptr, 0));
        break;

    case 'e':                 //* Equalizer
        strtol(msg, &ptr, 0); //* Bass
        strtol(msg, &ptr, 0); //* Mid
        strtol(msg, &ptr, 0); //* Treble
        break;

    case 's': //* SD card
        sd_card_toggle();
        break;

    case 'b': //* Bone Conductors
        i2s_toggle_bone_conductors();
        break;

    default:
        ESP_LOGE(MAIN_TAG, "Unknown message (%s)", msg);
        break;
    }

    spp_send_msg(SPP_OK); // Send response
}

void change_to_mode(int m)
{
    switch (m)
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
        i2s_turn_devices_off();
        bt_music_deinit();

        i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO); // Set 32 bit I2S
        microphones_init();

        ESP_LOGW(MAIN_TAG, "RECORD_PLAYBACK mode ready");
        break;

    default:
        break;
    }

    mode = m;

    spp_send_msg("m %d", mode);
}
