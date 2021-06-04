#include "i2s.h"

#include "gpio.h"
#include "sd.h"
#include "bt.h"
#include "dsp.h"

enum DEVICES
{
    BOTH_DEVICES,
    ONLY_BONE_CONDUCTORS,
    ONLY_SPEAKERS
};

static enum DEVICES devices = BOTH_DEVICES;

static int i2s0_device = NONE;

static bool i2s0_state = OFF;
static bool i2s1_state = OFF;

static xTaskHandle s_i2s_read_task_handle = NULL;
static void i2s_read_task(void *arg);
static void i2s_read_task_init();
static void i2s_read_task_deinit();

static void speakers_deinit();
static void microphones_deinit();

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
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 32 bit when playing back from microphones
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0, //Default interrupt priority
    .dma_buf_count = I2S_DMA_BUFFER_COUNT,
    .dma_buf_len = I2S_DMA_BUFFER_LEN,
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
    .dma_buf_count = I2S_DMA_BUFFER_COUNT,
    .dma_buf_len = I2S_DMA_BUFFER_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = true, //Auto clear tx descriptor on underflow
    .fixed_mclk = 0};

bool i2s_get_device_state(int device)
{
    if (device == SPEAKERS_MICROPHONES_I2S_NUM)
        return i2s0_state;
    if (device == BONE_CONDUCTORS_I2S_NUM)
        return i2s1_state;
    return false;
}

void i2s_set_device_state(int device, bool state)
{
    if (device == SPEAKERS_MICROPHONES_I2S_NUM)
    {
        if (state && i2s0_device == SPEAKERS)
            gpio_set_level(SPEAKERS_SD_PIN, HIGH); // Turn on speakers
        else
            gpio_set_level(SPEAKERS_SD_PIN, LOW); // Turn off speakers

        i2s0_state = state;
    }

    if (device == BONE_CONDUCTORS_I2S_NUM)
    {
        if (state)
            gpio_set_level(BONE_CONDUCTORS_SD_PIN, HIGH); // Turn on bone conductors
        else
            gpio_set_level(BONE_CONDUCTORS_SD_PIN, LOW); // Turn off bone conductors

        i2s1_state = state;
    }
}

void i2s_change_devices_state()
{
    switch (devices)
    {
    case BOTH_DEVICES:
        ESP_LOGI(I2S_TAG, "Change to only bone conductors");
        i2s_set_device_state(SPEAKERS_MICROPHONES_I2S_NUM, OFF);
        devices = ONLY_BONE_CONDUCTORS;
        break;

    case ONLY_BONE_CONDUCTORS:
        ESP_LOGI(I2S_TAG, "Change to only speakers");
        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, OFF);
        i2s_set_device_state(SPEAKERS_MICROPHONES_I2S_NUM, ON);
        devices = ONLY_SPEAKERS;
        break;

    case ONLY_SPEAKERS:
        ESP_LOGI(I2S_TAG, "Change to both devices");
        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, ON);
        devices = BOTH_DEVICES;
        break;
    }
}

void i2s_turn_devices_on()
{
    switch (devices)
    {
    case BOTH_DEVICES:
        i2s_set_device_state(SPEAKERS_MICROPHONES_I2S_NUM, ON);
        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, ON);
        break;

    case ONLY_SPEAKERS:
        i2s_set_device_state(SPEAKERS_MICROPHONES_I2S_NUM, ON);
        break;

    case ONLY_BONE_CONDUCTORS:
        i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, ON);
        break;
    }
}
void i2s_turn_devices_off()
{
    i2s_zero_dma_buffer(SPEAKERS_MICROPHONES_I2S_NUM);
    i2s_zero_dma_buffer(BONE_CONDUCTORS_I2S_NUM);

    i2s_set_device_state(SPEAKERS_MICROPHONES_I2S_NUM, OFF);
    i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, OFF);
}

void speakers_init()
{
    if (i2s0_device == SPEAKERS)
        return;

    microphones_deinit();

    i2s_driver_install(SPEAKERS_MICROPHONES_I2S_NUM, &i2s_config_tx, 0, NULL);
    i2s_set_pin(SPEAKERS_MICROPHONES_I2S_NUM, &speakers_pin_config);

    //i2s_zero_dma_buffer(SPEAKERS_MICROPHONES_I2S_NUM);

    i2s0_device = SPEAKERS;

    i2s_set_device_state(SPEAKERS_MICROPHONES_I2S_NUM, OFF);
}
static void speakers_deinit()
{
    if (i2s0_device != SPEAKERS)
        return;

    i2s_set_device_state(SPEAKERS_MICROPHONES_I2S_NUM, OFF);
    i2s0_device = NONE;

    //i2s_zero_dma_buffer(SPEAKERS_MICROPHONES_I2S_NUM);

    delay(DEVICE_DEINIT_DELAY);
    i2s_driver_uninstall(SPEAKERS_MICROPHONES_I2S_NUM);
}

void microphones_init()
{
    if (i2s0_device == MICROPHONES)
        return;

    speakers_deinit();

    i2s_driver_install(SPEAKERS_MICROPHONES_I2S_NUM, &i2s_config_rx, 0, NULL);
    i2s_set_pin(SPEAKERS_MICROPHONES_I2S_NUM, &microphones_pin_config);

    i2s_read_task_init();

    i2s0_device = MICROPHONES;
    i2s_set_device_state(SPEAKERS_MICROPHONES_I2S_NUM, ON);
}
static void microphones_deinit()
{
    if (i2s0_device != MICROPHONES)
        return;

    i2s_set_device_state(SPEAKERS_MICROPHONES_I2S_NUM, OFF);
    i2s0_device = NONE;

    i2s_read_task_deinit();

    delay(DEVICE_DEINIT_DELAY);
    i2s_driver_uninstall(SPEAKERS_MICROPHONES_I2S_NUM);
}

void bone_conductors_init()
{
    i2s_driver_install(BONE_CONDUCTORS_I2S_NUM, &i2s_config_tx, 0, NULL);
    i2s_set_pin(BONE_CONDUCTORS_I2S_NUM, &bone_conductors_pin_config);

    //i2s_zero_dma_buffer(BONE_CONDUCTORS_I2S_NUM);

    i2s_set_device_state(BONE_CONDUCTORS_I2S_NUM, OFF);
}
/*void bone_conductors_deinit()
{
    if (i2s1_device != BONE_CONDUCTORS)
        return;
    i2s1_device = NONE;
    //i2s_zero_dma_buffer(BONE_CONDUCTORS_I2S_NUM);
    delay(DEVICE_DEINIT_DELAY);
    i2s_driver_uninstall(BONE_CONDUCTORS_I2S_NUM);
}*/

static uint8_t *bone_conductors_samples;
static uint8_t *speaker_samples;

void i2s_init()
{
    bone_conductors_init();
    speakers_init();

    bone_conductors_samples = (uint8_t *)pvPortMalloc(DATA_LENGTH);
    speaker_samples = (uint8_t *)pvPortMalloc(DATA_LENGTH);
}

void i2s_write_data(uint8_t *data, size_t *len)
{
    //! I2S writes data with stereo inverted

    /*int16_t temp;
    for (size_t i = 0; i < *len; i += 2)
    {
        temp = data[i];
        data[i] = data[i + 1];
        data[i + 1] = temp;
    }*/

    memcpy(bone_conductors_samples, data, *len);
    memcpy(speaker_samples, data, *len);

    if (devices == BOTH_DEVICES && i2s0_device == SPEAKERS)
        apply_crossover(data, bone_conductors_samples, speaker_samples, len);

    size_t bytes_written = 0;

    int64_t tick_1, tick_2, tick_3;
    if (I2S_DEBUG)
    {
        printf("Length: %d\n", *len);
        tick_1 = esp_timer_get_time();
    }

    if (i2s0_state && i2s0_device == SPEAKERS) // If speakers are on
        i2s_write(SPEAKERS_MICROPHONES_I2S_NUM, speaker_samples, *len, &bytes_written, portMAX_DELAY);

    if (I2S_DEBUG)
    {
        tick_2 = esp_timer_get_time();
        printf("After writing first i2s: +%lldus\n", tick_2 - tick_1);
    }

    if (i2s1_state) // If bone conductors are on
        i2s_write(BONE_CONDUCTORS_I2S_NUM, bone_conductors_samples, *len, &bytes_written, portMAX_DELAY);

    if (I2S_DEBUG)
    {
        tick_3 = esp_timer_get_time();
        printf("After writing second i2s: +%lldus\n", tick_3 - tick_2);

        printf("Total time to write to i2s: %lldus\n\n", tick_3 - tick_1);
    }

    sd_write_data(bone_conductors_samples, len); //! Testing
}

static void i2s_read_task(void *arg)
{
    size_t bytes_read = 0;
    uint8_t data[DATA_LENGTH] = {0};

    for (;;)
    {
        if (!i2s1_state && !sd_file_state())
        {
            delay(READ_TASK_IDLE_DELAY);
            continue;
        }

        i2s_read(SPEAKERS_MICROPHONES_I2S_NUM, data, DATA_LENGTH, &bytes_read, portMAX_DELAY);

        i2s_write_data(data, &bytes_read);

        sd_write_data(data, &bytes_read);
    }

    s_i2s_read_task_handle = NULL;
    vTaskDelete(NULL);
}
static void i2s_read_task_init()
{
    if (!s_i2s_read_task_handle)
        xTaskCreate(i2s_read_task, "i2s_read_task", I2S_READ_STACK_DEPTH, NULL, configMAX_PRIORITIES - 3, &s_i2s_read_task_handle);
}
static void i2s_read_task_deinit()
{
    if (s_i2s_read_task_handle)
    {
        vTaskDelete(s_i2s_read_task_handle);
        s_i2s_read_task_handle = NULL;
    }
}