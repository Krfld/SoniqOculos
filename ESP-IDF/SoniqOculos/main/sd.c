#include "sd.h"

#include "gpio.h"
#include "bt.h"

#define SD_CARD_TAG "SD_CARD"

typedef struct wav_header_s
{
    uint8_t riff[4]; // 'RIFF' string
    uint32_t size;   // size of file in bytes
    uint8_t wave[4]; // 'WAVE' string

    uint8_t fmt[4];           // 'fmt ' string with trailing null char
    uint32_t fmt_size;        // length of the format data
    uint16_t fmt_type;        // format type. 1-PCM, 3-IEEE float, 6-8bit A law, 7-8bit mu law
    uint16_t channels;        // n. of channels
    uint32_t sample_rate;     // sampling rate
    uint32_t byte_rate;       // SampleRate * NumChannels * BitsPerSample/8
    uint16_t block_align;     // NumChannels * BitsPerSample/8
    uint16_t bits_per_sample; // bits per sample, 8 - 8bits, 16 - 16 bits etc

    uint8_t data[4];    // 'data' string
    uint32_t data_size; // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} wav_header_t;

static sdmmc_card_t *card;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static FILE *f = NULL;
static wav_header_t wav_header;

static SemaphoreHandle_t sd_semaphore_handle;

static void wav_header_init();

bool sd_card_state()
{
    return card != NULL && f != NULL;
}

static void wav_header_init()
{
    wav_header.riff[0] = 'R';
    wav_header.riff[1] = 'I';
    wav_header.riff[2] = 'F';
    wav_header.riff[3] = 'F';
    wav_header.size = sizeof(wav_header_t) - 8;
    wav_header.wave[0] = 'W';
    wav_header.wave[1] = 'A';
    wav_header.wave[2] = 'V';
    wav_header.wave[3] = 'E';

    wav_header.fmt[0] = 'f';
    wav_header.fmt[1] = 'm';
    wav_header.fmt[2] = 't';
    wav_header.fmt[3] = ' ';
    wav_header.fmt_size = FMT_SIZE;
    wav_header.fmt_type = AUDIO_FORMAT;
    wav_header.channels = NUM_CHANNELS;
    wav_header.sample_rate = SAMPLE_RATE;
    wav_header.byte_rate = BYTE_RATE;
    wav_header.block_align = BLOCK_ALIGN;
    wav_header.bits_per_sample = BITS_PER_SAMPLE;

    wav_header.data[0] = 'd';
    wav_header.data[1] = 'a';
    wav_header.data[2] = 't';
    wav_header.data[3] = 'a';
    wav_header.data_size = 0;
}

void spi_init()
{
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_MOSI_PIN,
        .miso_io_num = SD_MISO_PIN,
        .sclk_io_num = SD_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };

    spi_bus_initialize(host.slot, &bus_cfg, 1);

    sd_semaphore_handle = xSemaphoreCreateBinary();
    if (!sd_semaphore_handle)
        ESP_LOGE(SD_CARD_TAG, "Error creating semaphore");
    xSemaphoreGive(sd_semaphore_handle);
}

void sd_card_init()
{
    if (sd_card_state())
        return;

    if (!get_sd_det_state())
    {
        ESP_LOGW(SD_CARD_TAG, "Insert card");
        return;
    }

    ESP_LOGI(SD_CARD_TAG, "Start recording");

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 1,
        .allocation_unit_size = 16 * 1024};

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = host.slot;

    if (esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card) != ESP_OK) // Mount card
    {
        ESP_LOGW(SD_CARD_TAG, "Reinsert card");
        return;
    }

    ESP_LOGI(SD_CARD_TAG, "Card mounted");

    sd_open_file(FILE_NAME); // Open file

    ESP_LOGW(SD_CARD_TAG, "Free heap: %d", esp_get_free_heap_size());
}

void sd_card_deinit()
{
    if (!sd_card_state())
        return;

    ESP_LOGI(SD_CARD_TAG, "Stop recording");

    xSemaphoreTake(sd_semaphore_handle, portMAX_DELAY); // Can´t deinit when writing to file

    if (!get_sd_det_state()) // Check if card was removed unexpectedly
    {
        ESP_LOGE(SD_CARD_TAG, "SD fault");
        wav_header.data_size = 0;
    }

    sd_close_file();

    if (card == NULL)
        return;

    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    card = NULL;

    xSemaphoreGive(sd_semaphore_handle);

    ESP_LOGI(SD_CARD_TAG, "Card unmounted");
    ESP_LOGW(SD_CARD_TAG, "Safe to remove card");

    ESP_LOGW(SD_CARD_TAG, "Free heap: %d", esp_get_free_heap_size());
}

void sd_open_file(char *file)
{
    if (card == NULL)
        return;

    if (f != NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "File already opened");
        return;
    }

    char filename[64];
    int32_t file_number = nvs_read(FILE_NAME);
    sprintf(filename, MOUNT_POINT "/%s_%d.wav", file, file_number);

    f = fopen(filename, WRITE); // Open file
    if (f == NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "Failed to open file");
        sd_card_deinit();
        return;
    }

    wav_header_init();                               // Reset wav header
    fwrite(&wav_header, sizeof(wav_header_t), 1, f); // Write wav header

    ESP_LOGI(SD_CARD_TAG, "File opened '%s'", filename);
}

void sd_close_file()
{
    if (f == NULL)
        return;

    if (wav_header.data_size > 0)
    {
        // Rewrite wav header
        fseek(f, 0, SEEK_SET);
        fwrite(&wav_header, sizeof(wav_header_t), 1, f);

        nvs_write(FILE_NAME, nvs_read(FILE_NAME) + 1); // Increment file ID if not empty
    }

    fclose(f);
    f = NULL;
    ESP_LOGI(SD_CARD_TAG, "File closed");
}

void sd_write_data(uint8_t *data, size_t *len)
{
    if (f == NULL || !get_sd_det_state()) // Check if file was closed or card was removed
        return;

    if (!xSemaphoreTake(sd_semaphore_handle, portMAX_DELAY) || !sd_card_state())
    {
        xSemaphoreGive(sd_semaphore_handle);
        return;
    }

    fwrite(data, sizeof(*data), *len, f); // Write samples to file
    wav_header.size += *len;              // Increment wav size
    wav_header.data_size += *len;         // Increment wav data size

    xSemaphoreGive(sd_semaphore_handle);
}

void sd_set_card(bool state)
{
    vibrate(VIBRATION_DELAY);

    if (state)
        sd_card_init(); // Mount SD card and create file to write
    else
        sd_card_deinit(); // Close file and unmount SD card

    if (sd_card_state() != state) // Vibrate again if init failed
    {
        delay(100);
        vibrate(VIBRATION_DELAY);
    }

    spp_send_msg("r %d", sd_card_state());
}
