#include "main.h"

#include "bt.h"
#include "i2s.h"

void app_main(void)
{
    setup();

    //gpio_set_level(LED_BUILTIN, LOW);

    delay(1000);

    printf("Ready to connect\n");
    gpio_set_level(LED_BUILTIN, HIGH);
}

void setup()
{
    i2s_setup();

    bt_init();

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

void audioOnOff(bool state)
{
    if (state)
    {
        /// Right MAYBE NOT
        dac_output_voltage(RIGHT_CHANEL, 1.2 / LOGIC * 255);
        /// Left MAYBE NOT
        dac_output_voltage(LEFT_CHANEL, 1.6 / LOGIC * 255);
    }
    else
    {
        dac_output_voltage(RIGHT_CHANEL, LOW);
        dac_output_voltage(LEFT_CHANEL, LOW);
    }
}

void handleMsgs(char *msg, size_t len)
{
    printf("%s\n", msg);

    // Response
    sprintf(msg, "Message received\n");
}

void process_data(uint8_t *data, size_t len)
{
    size_t i2s0_bytes_written = 0, i2s1_bytes_written = 0;

    /*for (size_t i = 0; i < len; i += 4)
    {
        printf("Left %c ", (signed char)data[i]);
        printf("%c\n", (signed char)data[i + 1]);

        printf("Right %c ", (signed char)data[i + 2]);
        printf("%c\n", (signed char)data[i + 3]);
    }*/

    //TODO Write to i2s
    i2s_write(BONE_CONDUCTORS, data, len, &i2s0_bytes_written, portMAX_DELAY);
    i2s_write(SPEAKERS, data, len, &i2s1_bytes_written, portMAX_DELAY);
}