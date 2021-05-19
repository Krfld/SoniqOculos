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
    spi_init();

    //dac_output_enable(DAC_CHANNEL_1);
    //dac_output_voltage(DAC_CHANNEL_1, 3.2 / LOGIC * UCHAR_MAX);

    speakers_init();
    bone_conductors_init();

    bt_init();
    sd_det_task_init();
    gpio_task_init();

    //* Proccessing
    fir_f32_t fir;
    //dsps_fir_init_f32(fir, );

    printf("\nSetup ready\n\n");
}

void handleMsgs(char *msg)
{
    //TODO Implement with flutter

    printf("\n%s\n\n", msg);

    // Response
    sprintf(msg, "Message received\n");
}

static int buffer_index = 0;
static int16_t buffer[1024];

void process_data(uint8_t *data, size_t *len)
{
    //dsps_fir_f32_ae32();

    int16_t *samples_left = {0};
    int16_t *samples_right = {0};

    int sample_amount = *len / 4;
    for (size_t i = 0; i < sample_amount; i++)
    {
        samples_left[i] = (int16_t)(data[i * 4 + 1] | data[i * 4]);
        samples_right[i] = (int16_t)(data[i * 4 + 3] | data[i * 4 + 2]);
    }

    //TODO Process data
    i2s_write_data(data, len);

    //sd_write_data(data, len); //! Testing
}
