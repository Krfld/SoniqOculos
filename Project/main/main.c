#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"

void app_main(void)
{
    i2s_setup();

    bt_init();

    dac_output_enable(DAC_CHANNEL_1);
    dac_output_enable(DAC_CHANNEL_2);
    audioOnOff(ON);

    delay(1000);

    gpio_set(LED_BUILTIN, HIGH);
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
        dac_output_voltage(RIGHT_CHANEL, 1.2 / LOGIC * 255); // Right
        dac_output_voltage(LEFT_CHANEL, 1.6 / LOGIC * 255);  // Left
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

    i2s_write(BONE_CONDUCTORS_I2S_NUM, data, len, &i2s0_bytes_written, portMAX_DELAY);
    i2s_write(SPEAKERS_I2S_NUM, data, len, &i2s1_bytes_written, portMAX_DELAY);
}