#include "tools.h"

void setup()
{
    gpio_pad_select_gpio(LED_BUILTIN);
    gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);

    dac_output_enable(DAC_CHANNEL_1);
    dac_output_enable(DAC_CHANNEL_2);
    audioOnOff(ON);
}

void delay(int millis)
{
    vTaskDelay(millis / portTICK_RATE_MS);
}

void handleMsgs(char *msg, size_t len)
{
    printf("%s\n", msg);

    // Response
    sprintf(msg, "Message received\n");
}

void audioOnOff(bool state)
{
    if (state)
    {
        /// Right MAYBE NOT
        dac_output_voltage(RIGHT_CHANEL, 1.2 / VDD * 255);
        /// Left MAYBE NOT
        dac_output_voltage(LEFT_CHANEL, 1.6 / VDD * 255);
    }
    else
    {
        dac_output_voltage(RIGHT_CHANEL, LOW);
        dac_output_voltage(LEFT_CHANEL, LOW);
    }
}
