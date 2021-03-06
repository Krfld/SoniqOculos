#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include <math.h>

#define MICROPHONES_I2S_NUM I2S_NUM_0
#define MICROPHONES_WS_PIN 32
#define MICROPHONES_BCK_PIN 27
#define MICROPHONES_DATA_PIN 33

#define BONE_CONDUCTORS_I2S_NUM I2S_NUM_1
#define BONE_CONDUCTORS_WS_PIN 3
#define BONE_CONDUCTORS_BCK_PIN 21
#define BONE_CONDUCTORS_DATA_PIN 19

#define DMA_BUFFER_COUNT 8
#define DMA_BUFFER_LEN 1024

//* Bone conductors pin configuration
static i2s_pin_config_t bone_conductors_pin_config = {
    .ws_io_num = BONE_CONDUCTORS_WS_PIN,
    .bck_io_num = BONE_CONDUCTORS_BCK_PIN,
    .data_out_num = BONE_CONDUCTORS_DATA_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE};

//* Microphones pin configuration
static i2s_pin_config_t microphones_pin_config = {
    .ws_io_num = MICROPHONES_WS_PIN,
    .bck_io_num = MICROPHONES_BCK_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = MICROPHONES_DATA_PIN};

//* Output configuration
static i2s_config_t i2s_config_tx = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX,
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0, //Default interrupt priority
    .dma_buf_count = DMA_BUFFER_COUNT,
    .dma_buf_len = DMA_BUFFER_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = true, //Auto clear tx descriptor on underflow
    .fixed_mclk = 0};

//* Input configuration
static i2s_config_t i2s_config_rx = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX,
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0, //Default interrupt priority
    .dma_buf_count = DMA_BUFFER_COUNT,
    .dma_buf_len = DMA_BUFFER_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = true, //Auto clear tx descriptor on underflow
    .fixed_mclk = 0};

void app_main(void)
{
    // Bone conductors setup
    if (i2s_driver_install(BONE_CONDUCTORS_I2S_NUM, &i2s_config_tx, 0, NULL) != ESP_OK)
        printf("Bone conductors i2s driver install failed\n");

    if (i2s_set_pin(BONE_CONDUCTORS_I2S_NUM, &bone_conductors_pin_config) != ESP_OK)
        printf("Bone conductors i2s set pin failed\n");

    // Microphones setup
    if (i2s_driver_install(MICROPHONES_I2S_NUM, &i2s_config_rx, 0, NULL) != ESP_OK)
        printf("Microphones i2s driver install failed\n");

    if (i2s_set_pin(MICROPHONES_I2S_NUM, &microphones_pin_config) != ESP_OK)
        printf("Microphones i2s set pin failed\n");

    size_t bytes_read = 0, bytes_written = 0;
    uint8_t data[DMA_BUFFER_LEN] = {0};

    for (;;)
    {
        i2s_read(MICROPHONES_I2S_NUM, data, sizeof(data), &bytes_read, portMAX_DELAY);

        i2s_write(BONE_CONDUCTORS_I2S_NUM, data, sizeof(data), &bytes_written, portMAX_DELAY);
    }
}
