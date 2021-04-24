#include "i2s.h"

static xTaskHandle s_i2s_task_handle = NULL;

void i2s_setup()
{
    i2s_config_t i2s_config_output = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX, // Only TX
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0, //Default interrupt priority
        .dma_buf_count = DMA_BUFFER_COUNT,
        .dma_buf_len = DMA_BUFFER_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = true, //Auto clear tx descriptor on underflow
        .fixed_mclk = 0};

    /// I2S0
    if (i2s_driver_install(SPEAKERS, &i2s_config_output, 0, NULL) != ESP_OK)
        printf("I2S0 Driver install failed\n");

    i2s_pin_config_t i2s0_pin_config = {
        .ws_io_num = I2S_WS_PIN(SPEAKERS),
        .bck_io_num = I2S_BCK_PIN(SPEAKERS),
        .data_out_num = I2S_DATA_PIN(SPEAKERS),
        .data_in_num = I2S_PIN_NO_CHANGE};
    if (i2s_set_pin(SPEAKERS, &i2s0_pin_config) != ESP_OK)
        printf("I2S0 Set pin failed\n");

    /// I2S1
    if (i2s_driver_install(BONE_CONDUCTORS, &i2s_config_output, 0, NULL) != ESP_OK)
        printf("I2S1 Driver install failed\n");

    i2s_pin_config_t i2s1_pin_config = {
        .ws_io_num = I2S_WS_PIN(BONE_CONDUCTORS),
        .bck_io_num = I2S_BCK_PIN(BONE_CONDUCTORS),
        .data_out_num = I2S_DATA_PIN(BONE_CONDUCTORS),
        .data_in_num = I2S_PIN_NO_CHANGE};
    if (i2s_set_pin(BONE_CONDUCTORS, &i2s1_pin_config) != ESP_OK)
        printf("I2S1 Set pin failed\n");
}

//TODO Configure input

static void i2s_task_handler(void *arg)
{
    size_t bytesRead = 0;
    uint8_t buffer32[DMA_BUFFER_LEN] = {0};

    for (;;)
    {
        i2s_read(MICROPHONES, &buffer32, sizeof(buffer32), &bytesRead, 100);
    }
}

void i2s_task_start_up(void)
{
    xTaskCreate(i2s_task_handler, "I2ST", 1024, NULL, configMAX_PRIORITIES - 3, &s_i2s_task_handle);
    return;
}

void i2s_task_shut_down(void)
{
    if (s_i2s_task_handle)
    {
        vTaskDelete(s_i2s_task_handle);
        s_i2s_task_handle = NULL;
    }
}
