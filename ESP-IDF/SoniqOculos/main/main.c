#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"
#include "sd.h"

#include "dsp.h"

void app_main(void)
{
    ESP_LOGW(MAIN_TAG, "Wakeup cause: %d", esp_sleep_get_wakeup_cause()); // 2 - ESP_SLEEP_WAKEUP_EXT0

    ESP_LOGW(MAIN_TAG, "Setup init");

    bt_init();
    spi_init();
    sd_det_task_init();
    gpio_task_init();

    crossover_init();

    bone_conductors_init();
    speakers_init();
    bt_music_init();

    ESP_LOGW(MAIN_TAG, "Setup ready");
}

void delay(int millis)
{
    vTaskDelay(pdMS_TO_TICKS(millis));
}

void handleMsgs(char *msg)
{
    //TODO Implement with flutter

    ESP_LOGW(BT_SPP_TAG, "Message recevied %s", msg);

    // Response
    sprintf(msg, "Message received\n");
}
