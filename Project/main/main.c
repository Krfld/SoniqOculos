#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"
#include "sd.h"

void app_main(void)
{
    sd_init();

    i2s_setup();
    bt_init();

    dac_output_enable(DAC_CHANNEL_1);
    dac_output_enable(DAC_CHANNEL_2);
    audioOnOff(ON);

    printf("\nReady to connect\n\n");
}

void delay(int millis)
{
    vTaskDelay(millis / portTICK_RATE_MS);
}

void audioOnOff(bool state)
{
    if (state)
    {
        dac_output_voltage(RIGHT_CHANEL, 1.2 / LOGIC * UCHAR_MAX); // Right
        dac_output_voltage(LEFT_CHANEL, 1.6 / LOGIC * UCHAR_MAX);  // Left
    }
    else
    {
        dac_output_voltage(RIGHT_CHANEL, LOW);
        dac_output_voltage(LEFT_CHANEL, LOW);
    }
}

void handleMsgs(char *msg, size_t len)
{
    printf("\n%s\n\n", msg);

    // Response
    sprintf(msg, "Message received\n");
}

void process_data(uint8_t *data, size_t *len)
{
    size_t i2s0_bytes_written = 0, i2s1_bytes_written = 0;

    i2s_write(BONE_CONDUCTORS_I2S_NUM, data, *len, &i2s0_bytes_written, portMAX_DELAY);
    i2s_write(SPEAKERS_I2S_NUM, data, *len, &i2s1_bytes_written, portMAX_DELAY);
}
