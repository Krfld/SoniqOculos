#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"

void app_main(void)
{
    //! Testing
    i2s_setup();

    dac_output_enable(DAC_CHANNEL_1);
    dac_output_enable(DAC_CHANNEL_2);
    audioOnOff(ON);

    gpio_direction(LED_BUILTIN);
    gpio_set_level(LED_BUILTIN, HIGH);
    printf("Ready\n");

    size_t bytes_read = 0, bytes_written = 0;
    uint8_t buffer[DMA_BUFFER_LEN] = {0};

    for (;;)
    {
        i2s_read(MICROPHONES_I2S_NUM, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
        /*for (size_t i = 0; i < bytes_read; i++)
        {
            printf("%d\n", buffer[i]);
        }*/

        i2s_write(BONE_CONDUCTORS_I2S_NUM, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
    }
    //!

    /*i2s_setup();

    bt_init();

    gpio_direction(LED_BUILTIN);
    gpio_set_level(LED_BUILTIN, HIGH);

    dac_output_enable(DAC_CHANNEL_1);
    dac_output_enable(DAC_CHANNEL_2);
    audioOnOff(ON);

    delay(1000);

    gpio_set_level(LED_BUILTIN, HIGH);
    printf("Ready to connect\n");*/
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

    i2s_write(BONE_CONDUCTORS_I2S_NUM, data, len, &i2s0_bytes_written, portMAX_DELAY);
    i2s_write(SPEAKERS_I2S_NUM, data, len, &i2s1_bytes_written, portMAX_DELAY);
}