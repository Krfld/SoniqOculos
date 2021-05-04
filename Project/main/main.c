#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"
#include "sd.h"

void delay(int millis)
{
    vTaskDelay(millis / portTICK_RATE_MS);
}

void app_main(void)
{
    gpio_init();

    bt_init();

    set_mode(MUSIC);

    //sd_init();

    dac_output_enable(DAC_CHANNEL_1);
    dac_output_enable(DAC_CHANNEL_2);
    audioOnOff(ON);

    printf("\nReady to connect\n\n");
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

void handleMsgs(char *msg)
{
    printf("\n%s\n\n", msg);
    //TODO Change mode

    // Response
    sprintf(msg, "Message received\n");
}

void process_data(uint8_t *data, size_t *len)
{
    //TODO Process data
    i2s_write_data(data, len);
}
