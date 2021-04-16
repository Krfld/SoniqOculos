#include "tools.h"

void delay(int millis)
{
    vTaskDelay(millis / portTICK_RATE_MS);
}

void tools()
{
    i2s_setup();
}

void i2s_setup()
{
    i2s_config_t i2s_output_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .sample_rate = 44100,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .intr_alloc_flags = 0, // default interrupt priority
        .tx_desc_auto_clear = true,
        .use_apll = false};

    i2s_driver_install(I2S_NUM_0, &i2s_output_config, 0, NULL);
    i2s_pin_config_t i2s0_pin_config = {
        .ws_io_num = I2S_WS_PIN(0),
        .bck_io_num = I2S_WS_PIN(0),
        .data_out_num = I2S_WS_PIN(0),
        .data_in_num = I2S_PIN_NO_CHANGE};
    i2s_set_pin(I2S_NUM_0, &i2s0_pin_config);

    i2s_driver_install(I2S_NUM_1, &i2s_output_config, 0, NULL);
    i2s_pin_config_t i2s1_pin_config = {
        .ws_io_num = I2S_WS_PIN(1),
        .bck_io_num = I2S_WS_PIN(1),
        .data_out_num = I2S_WS_PIN(1),
        .data_in_num = I2S_PIN_NO_CHANGE};
    i2s_set_pin(I2S_NUM_1, &i2s1_pin_config);
}