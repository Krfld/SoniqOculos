#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"
#include "sd.h"

void delay(int millis)
{
    vTaskDelay(millis / portTICK_PERIOD_MS);
}

void app_main(void)
{
    bt_init();
    spi_init();
    sd_det_task_init();
    gpio_task_init();

    speakers_init();
    bone_conductors_init();
    bt_music_init();

    ESP_LOGW(MAIN_TAG, "Setup ready");
}

void handleMsgs(char *msg)
{
    //TODO Implement with flutter

    ESP_LOGW(BT_SPP_TAG, "Message recevied %s", msg);

    // Response
    sprintf(msg, "Message received\n");
}

//static int buffer_index = 0;
//static int16_t buffer[1024];

void process_data(uint8_t *data, size_t *len)
{
    int16_t *samples = (int16_t *)data; // [0] - Left | [1] - Right | [2] - Left | [3] - Right ...

    //dsps_fir_f32_ae32(fir,);

    //TODO Process data
    i2s_write_data(data, len);

    //sd_write_data(data, len); //! Testing
}
