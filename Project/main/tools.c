#include "tools.h"

void delay(int millis)
{
    vTaskDelay(millis / portTICK_RATE_MS);
}

void tools()
{
    printf("START\n");
    i2s_setup();
}

void process_data(const uint8_t *data, size_t len)
{
    //size_t i2s0_bytes_written = 0, i2s1_bytes_written = 0;

    /*for (size_t i = 0; i < len; i += 4)
    {
        printf("Left %c ", (signed char)data[i]);
        printf("%c\n", (signed char)data[i + 1]);

        printf("Right %c ", (signed char)data[i + 2]);
        printf("%c\n", (signed char)data[i + 3]);
    }*/

    //TODO size_t bytes_written = 0; to i2s_write(0, data, item_size, &bytes_written, portMAX_DELAY);
    //TODO Write to i2s
}

void i2s_setup()
{
    i2s_config_t i2s_output_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .intr_alloc_flags = 0, // default interrupt priority
        .tx_desc_auto_clear = true,
        .use_apll = false};

    if (i2s_driver_install(I2S_NUM_0, &i2s_output_config, 0, NULL) != ESP_OK)
        printf("I2S0 Driver install failed\n");

    i2s_pin_config_t i2s0_pin_config = {
        .ws_io_num = I2S_WS_PIN(I2S_NUM_0),
        .bck_io_num = I2S_BCK_PIN(I2S_NUM_0),
        .data_out_num = I2S_DATA_PIN(I2S_NUM_0),
        .data_in_num = I2S_PIN_NO_CHANGE};
    if (i2s_set_pin(I2S_NUM_0, &i2s0_pin_config) != ESP_OK)
        printf("I2S0 Set pin failed\n");

    if (i2s_driver_install(I2S_NUM_1, &i2s_output_config, 0, NULL) != ESP_OK)
        printf("I2S1 Driver install failed\n");

    i2s_pin_config_t i2s1_pin_config = {
        .ws_io_num = I2S_WS_PIN(I2S_NUM_1),
        .bck_io_num = I2S_BCK_PIN(I2S_NUM_1),
        .data_out_num = I2S_DATA_PIN(I2S_NUM_1),
        .data_in_num = I2S_PIN_NO_CHANGE};
    if (i2s_set_pin(I2S_NUM_1, &i2s1_pin_config) != ESP_OK)
        printf("I2S1 Set pin failed\n");
}