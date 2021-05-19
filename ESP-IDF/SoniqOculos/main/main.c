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

    bt_init();
    sd_det_task_init();
    gpio_task_init();

    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1, 3.2 / LOGIC * UCHAR_MAX);

    speakers_init();
    bone_conductors_init();

    printf("\nSetup ready\n\n");
}

void handleMsgs(char *msg)
{
    //TODO Implement with flutter

    printf("\n%s\n\n", msg);

    // Response
    sprintf(msg, "Message received\n");
}

void process_data(uint8_t *data, size_t *len)
{
    //TODO Process data
    i2s_write_data(data, len);

    //sd_write_data(data, len); //! Testing
}
