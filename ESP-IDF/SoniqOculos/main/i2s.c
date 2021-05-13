#include "i2s.h"

#include "gpio.h"
#include "sd.h"
#include "bt.h"

enum DEVICE
{
    NONE,
    SPEAKERS,
    MICROPHONES,
    BONE_CONDUCTORS
};

static int i2s0_device = NONE;
static int i2s1_device = NONE;

static xTaskHandle s_i2s_read_task_handle = NULL;
static void i2s_read_task_handler(void *arg);

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
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 32 bit when playback from microphones
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
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // Microphones only work with 32 bit
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0, //Default interrupt priority
    .dma_buf_count = DMA_BUFFER_COUNT,
    .dma_buf_len = DMA_BUFFER_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = true, //Auto clear tx descriptor on underflow
    .fixed_mclk = 0};

void speakers_init()
{
    if (i2s0_device == SPEAKERS)
        return;

    microphones_deinit();

    if (i2s_driver_install(SPEAKERS_I2S_NUM, &i2s_config_tx, 0, NULL) != ESP_OK)
        printf("\nSpeakers i2s driver install failed\n\n");

    if (i2s_set_pin(SPEAKERS_I2S_NUM, &speakers_pin_config) != ESP_OK)
        printf("\nSpeakers i2s set pin failed\n\n");

    i2s_zero_dma_buffer(SPEAKERS_I2S_NUM);

    i2s0_device = SPEAKERS;
}
void speakers_deinit()
{
    if (i2s0_device != SPEAKERS)
        return;

    i2s0_device = NONE;

    i2s_zero_dma_buffer(SPEAKERS_I2S_NUM);

    delay(I2S_DEINIT_DELAY);

    if (i2s_driver_uninstall(SPEAKERS_I2S_NUM) != ESP_OK)
        printf("\nSpeakers i2s driver uninstall failed\n\n");

    i2s_pins_reset(SPEAKERS_WS_PIN, SPEAKERS_BCK_PIN, SPEAKERS_DATA_PIN);
}

void microphones_init()
{
    if (i2s0_device == MICROPHONES)
        return;

    speakers_deinit();

    if (i2s_driver_install(MICROPHONES_I2S_NUM, &i2s_config_rx, 0, NULL) != ESP_OK)
        printf("\nMicrophones i2s driver install failed\n\n");

    if (i2s_set_pin(MICROPHONES_I2S_NUM, &microphones_pin_config) != ESP_OK)
        printf("\nMicrophones i2s set pin failed\n\n");

    i2s0_device = MICROPHONES;

    if (!s_i2s_read_task_handle)
        xTaskCreate(i2s_read_task_handler, "i2s_read_task", I2S_STACK_DEPTH, NULL, configMAX_PRIORITIES - 3, &s_i2s_read_task_handle);
}
void microphones_deinit()
{
    if (i2s0_device != MICROPHONES)
        return;

    i2s0_device = NONE;

    //delay(I2S_DEINIT_DELAY);

    if (s_i2s_read_task_handle)
    {
        vTaskDelete(s_i2s_read_task_handle);
        s_i2s_read_task_handle = NULL;
    }

    if (i2s_driver_uninstall(MICROPHONES_I2S_NUM) != ESP_OK)
        printf("\nMicrophones i2s driver uninstall failed\n\n");

    i2s_pins_reset(MICROPHONES_WS_PIN, MICROPHONES_BCK_PIN, MICROPHONES_DATA_PIN);
}

void bone_conductors_init()
{
    if (i2s1_device == BONE_CONDUCTORS)
        return;

    if (i2s_driver_install(BONE_CONDUCTORS_I2S_NUM, &i2s_config_tx, 0, NULL) != ESP_OK)
        printf("\nBone conductors i2s driver install failed\n\n");

    if (i2s_set_pin(BONE_CONDUCTORS_I2S_NUM, &bone_conductors_pin_config) != ESP_OK)
        printf("\nBone conductors i2s set pin failed\n\n");

    i2s_zero_dma_buffer(BONE_CONDUCTORS_I2S_NUM);

    i2s1_device = BONE_CONDUCTORS;
}
void bone_conductors_deinit()
{
    if (i2s1_device != BONE_CONDUCTORS)
        return;

    i2s1_device = NONE;

    i2s_zero_dma_buffer(BONE_CONDUCTORS_I2S_NUM);

    delay(I2S_DEINIT_DELAY);

    if (i2s_driver_uninstall(BONE_CONDUCTORS_I2S_NUM) != ESP_OK)
        printf("\nBone conductors i2s driver uninstall failed\n\n");

    i2s_pins_reset(BONE_CONDUCTORS_WS_PIN, BONE_CONDUCTORS_BCK_PIN, BONE_CONDUCTORS_DATA_PIN);
}

void set_mode(int mode)
{
    //TODO Shutdown unused amps and handle SD Card
    /*if (mode != RECORD)
        sd_close_file();*/

    i2s0_device = NONE;
    i2s1_device = NONE;

    switch (mode)
    {
    case MUSIC:
        microphones_deinit(); // No need

        speakers_init();
        bone_conductors_init();
        bt_music_init();

        printf("\nMusic mode ready\n\n");
        break;
    case MUSIC_ISOLATED:
        speakers_deinit();
        microphones_deinit();

        bone_conductors_init();
        bt_music_init();

        printf("\nMusic Isolated mode ready\n\n");
        break;
    case MUSIC_SPEAKERS:
        microphones_deinit(); // No need
        bone_conductors_deinit();

        speakers_init();
        bt_music_init();

        printf("\nMusic Speakers mode ready\n\n");
        break;

    case PLAYBACK:
        bt_music_deinit();
        speakers_deinit(); // No need

        bone_conductors_init();
        i2s_set_clk(BONE_CONDUCTORS_I2S_NUM, 44100, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO);
        microphones_init();

        printf("\nPlayback mode ready\n\n");
        break;

    case RECORD: //TODO Handle SD card
        sd_init();

        bt_music_deinit();
        speakers_deinit(); // No need
        bone_conductors_deinit();

        microphones_init();

        printf("\nRecord mode ready\n\n");
        break;

    default: // IDLE Mode
        bt_music_deinit();
        speakers_deinit();
        microphones_deinit();
        bone_conductors_deinit();

        printf("\nIdle mode ready\n\n");
        break;
    }
}

void i2s_write_data(uint8_t *data, size_t *len)
{
    size_t i2s0_bytes_written = 0, i2s1_bytes_written = 0;

    int64_t tick_1, tick_2, tick_3;
    if (I2S_DEBUG)
    {
        printf("Length: %d\n", *len);
        tick_1 = esp_timer_get_time();
    }

    if (i2s0_device == SPEAKERS)
        i2s_write(SPEAKERS_I2S_NUM, data, *len, &i2s0_bytes_written, portMAX_DELAY);

    if (I2S_DEBUG)
    {
        tick_2 = esp_timer_get_time();
        printf("After writing first i2s: +%lldus\n", tick_2 - tick_1);
    }

    if (i2s1_device == BONE_CONDUCTORS)
        i2s_write(BONE_CONDUCTORS_I2S_NUM, data, *len, &i2s1_bytes_written, portMAX_DELAY);

    if (I2S_DEBUG)
    {
        tick_3 = esp_timer_get_time();
        printf("After writing second i2s: +%lldus\n", tick_3 - tick_2);

        printf("Total time to write to i2s: %lldus\n\n", tick_3 - tick_1);
    }
}

static void i2s_read_task_handler(void *arg)
{
    size_t bytes_read = 0;
    uint8_t data[DMA_BUFFER_LEN] = {0};

    for (;;)
    {
        if (i2s0_device != MICROPHONES) // No need
            continue;

        i2s_read(MICROPHONES_I2S_NUM, &data, sizeof(data), &bytes_read, portMAX_DELAY);

        if (bytes_read == 0)
            continue;

        if (i2s1_device == BONE_CONDUCTORS) //* Playback mode
            i2s_write_data(data, &bytes_read);
        else if (i2s0_device == MICROPHONES) //* Record mode
            sd_write_data(data, &bytes_read);
    }
}
