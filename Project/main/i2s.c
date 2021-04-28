#include "i2s.h"

#include "gpio.h"

//* Speakers pin configuration
static i2s_pin_config_t speakers_pin_config = {
    .ws_io_num = SPEAKERS_WS_PIN,
    .bck_io_num = SPEAKERS_BCK_PIN,
    .data_out_num = SPEAKERS_DATA_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE};

//* Microphones pin configuration
static i2s_pin_config_t microphones_pin_config = {
    .ws_io_num = MICROPHONES_WS_PIN,
    .bck_io_num = MICROPHONES_BCK_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = MICROPHONES_DATA_PIN};

//* Bone conductors pin configuration
static i2s_pin_config_t bone_conductors_pin_config = {
    .ws_io_num = BONE_CONDUCTORS_WS_PIN,
    .bck_io_num = BONE_CONDUCTORS_BCK_PIN,
    .data_out_num = BONE_CONDUCTORS_DATA_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE};

//* Output configuration
static i2s_config_t i2s_config_tx = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX,
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 32 bit when playing from microphones
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

static xTaskHandle s_i2s_task_handle = NULL;

void i2s_setup()
{
    bone_conductors_setup();
    speakers_setup();
    //microphones_setup();
}

void speakers_setup()
{
    if (i2s_driver_uninstall(MICROPHONES_I2S_NUM) != ESP_OK)
        printf("\nMicrophones i2s driver uninstall failed\n\n");

    gpio_set(MICROPHONES_WS_PIN, LOW);
    gpio_set(MICROPHONES_BCK_PIN, LOW);
    gpio_set(MICROPHONES_DATA_PIN, LOW);

    if (i2s_driver_install(SPEAKERS_I2S_NUM, &i2s_config_tx, 0, NULL) != ESP_OK)
        printf("\nSpeakers i2s driver install failed\n\n");

    if (i2s_set_pin(SPEAKERS_I2S_NUM, &speakers_pin_config) != ESP_OK)
        printf("\nSpeakers i2s set pin failed\n\n");
}

void microphones_setup()
{
    if (i2s_driver_uninstall(SPEAKERS_I2S_NUM) != ESP_OK)
        printf("\nSpeakers i2s driver uninstall failed\n\n");

    gpio_set(SPEAKERS_WS_PIN, LOW);
    gpio_set(SPEAKERS_BCK_PIN, LOW);
    gpio_set(SPEAKERS_DATA_PIN, LOW);

    if (i2s_driver_install(MICROPHONES_I2S_NUM, &i2s_config_rx, 0, NULL) != ESP_OK)
        printf("\nMicrophones i2s driver install failed\n\n");

    if (i2s_set_pin(MICROPHONES_I2S_NUM, &microphones_pin_config) != ESP_OK)
        printf("\nMicrophones i2s set pin failed\n\n");
}

void bone_conductors_setup()
{
    if (i2s_driver_install(BONE_CONDUCTORS_I2S_NUM, &i2s_config_tx, 0, NULL) != ESP_OK)
        printf("\nBone conductors i2s driver install failed\n\n");

    if (i2s_set_pin(BONE_CONDUCTORS_I2S_NUM, &bone_conductors_pin_config) != ESP_OK)
        printf("\nBone conductors i2s set pin failed\n\n");
}

//TODO Configure input
static void i2s_task_handler(void *arg)
{
    size_t bytes_read = 0;
    uint8_t data[DMA_BUFFER_LEN] = {0};

    for (;;)
    {
        i2s_read(MICROPHONES_I2S_NUM, &data, sizeof(data), &bytes_read, portMAX_DELAY);
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
