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
 * measure power consumption (sin 1kHz max volume)
 * test white noise
 */

RTC_DATA_ATTR static int mode = MUSIC; // Keep value while in deep-sleep
int get_mode()
{
    return mode;
}

void app_main(void)
{
    ESP_LOGW(MAIN_TAG, "Wakeup cause: %d", esp_sleep_get_wakeup_cause()); // 2 - ESP_SLEEP_WAKEUP_EXT0

    nvs_init(); // Flash to store non-volatile data
    spi_init(); // SPI to comunicate with SD card

    bt_init();        // Start BT SPP server
    dsp_init();       // Alocate DSP variables
    i2s_init();       // Setup I2S interface
    gpio_task_init(); // Start task to handle GPIOs

    switch (mode)
    {
    case MUSIC:
        speakers_init(); // Setup speakers
        bt_music_init(); // Start A2DP for BT audio transmission
        ESP_LOGW(MAIN_TAG, "MUSIC mode ready");
        break;

    case RECORD:
        microphones_init(); // Setup microphones and task to read them
        ESP_LOGW(MAIN_TAG, "RECORD mode ready");
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

    gpio_task_deinit(); // Stop button handling

    i2s_turn_devices_off(); // Stop devices
    sd_card_deinit();       // Close SD file
    bt_music_deinit();      // Disconnect from device if connected

    // Disable BT components
    esp_bluedroid_disable();
    esp_bt_controller_disable();

    while (gpio_get_level(B1) || gpio_get_level(B2) || gpio_get_level(B3)) // Wait if user pressing any button
        delay(DEBOUNCE);

    delay(COMMAND_DELAY);

    // Setup deep-sleep and start it
    esp_sleep_enable_ext0_wakeup(B1, HIGH);
    rtc_gpio_pulldown_en(B1);
    esp_deep_sleep_start();
}

void handleMsgs(char *msg)
{
    ESP_LOGW(MAIN_TAG, "Message recevied: '%s'", msg);

    /*if (spp_get_sending_state())
    {
        if (strcmp(msg, SPP_OK) == 0)
            spp_set_sending_state(false); // Stop sending if got OK or FAIL
        else
            spp_send_msg(SPP_FAIL); // Error if msg received while sending
        return;
    }*/

    // Message received (examples) | 'm 0' | 'v 100' | 'e 0 0 0' |

    char *ptr;
    switch (*msg++)
    {
    case 'm': // Mode
        change_to_mode(strtol(msg, &ptr, 0));
        break;

    case 'v': // Volume
        set_volume(strtol(msg, &ptr, 0));
        break;

    case 'd': // Devices
        i2s_change_to_devices(strtol(msg, &ptr, 0));
        break;

    case 'e': // Equalizer
        set_equalizer(strtol(msg, &ptr, 0), strtol(ptr, &ptr, 0), strtol(ptr, &ptr, 0));
        break;

    case 'r': // SD card
        sd_set_card(strtol(msg, &ptr, 0));
        break;

    case 'p': // Bone Conductors
        i2s_set_bone_conductors(strtol(msg, &ptr, 0));
        break;

    default:
        ESP_LOGE(MAIN_TAG, "Unknown message (%s)", --msg);
        break;
    }
}

void change_to_mode(int m)
{
    i2s_turn_devices_off();

    switch (m)
    {
    case MUSIC:
        i2s_set_bone_conductors(OFF); // Stop playback | Vibrate
        sd_card_deinit();             // Close SD file if opened

        speakers_init(); // Start speakers and stop microphones
        bt_music_init(); // Start BT audio transmission

        ESP_LOGW(MAIN_TAG, "MUSIC mode ready");
        break;

    case RECORD:
        vibrate(VIBRATION_DELAY); // Vibrate

        bt_music_deinit(); // Stop BT audio transmission and disconnect from devices

        microphones_init(); // Start microphones and stop speakers

        ESP_LOGW(MAIN_TAG, "RECORD mode ready");
        break;

    default:
        break;
    }

    mode = m;

    vibrate(VIBRATION_DELAY); // Vibrate again to indicate mode has changed

    spp_send_msg("m %d", mode); // Update mode in the app if connected

    ESP_LOGW(MAIN_TAG, "Free heap: %d", esp_get_free_heap_size());
}
