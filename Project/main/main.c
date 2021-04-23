#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "tools.h"

void app_main(void)
{
    i2s_setup();

    bt_init();

    setup();

    //gpio_set_level(LED_BUILTIN, LOW);

    delay(1000);

    printf("Ready to connect\n");
    gpio_set_level(LED_BUILTIN, HIGH);
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
    i2s_write(I2S_NUM_0, data, len, &i2s0_bytes_written, portMAX_DELAY);
    i2s_write(I2S_NUM_1, data, len, &i2s1_bytes_written, portMAX_DELAY);
}