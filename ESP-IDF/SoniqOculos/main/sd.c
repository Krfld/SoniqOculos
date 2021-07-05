#include "sd.h"

#include "gpio.h"
#include "bt.h"

#define SD_CARD_TAG "SD_CARD"

static sdmmc_card_t *card;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static FILE *f = NULL;
static bool empty = true;

static SemaphoreHandle_t sd_semaphore_handle;

bool sd_card_state()
{
    return card != NULL && f != NULL;
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

    sd_semaphore_handle = xSemaphoreCreateMutex();
}

void sd_card_init()
{
    if (card != NULL)
        return;

    if (!get_sd_det_state())
    {
        ESP_LOGW(SD_CARD_TAG, "Insert card");
        return;
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 1,
        .allocation_unit_size = 16 * 1024};

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = host.slot;

    if (esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card) != ESP_OK)
    {
        ESP_LOGW(SD_CARD_TAG, "Reinsert card");
        return;
    }

    ESP_LOGI(SD_CARD_TAG, "Card mounted");

    sd_open_file(FILE_NAME, WRITE);

    ESP_LOGW(SD_CARD_TAG, "Free heap: %d", esp_get_free_heap_size());
}

void sd_card_deinit()
{
    xSemaphoreTake(sd_semaphore_handle, portMAX_DELAY); // Can´t deinit when writing to file

    if (sd_card_state()) // Check if card was removed unexpectedly
    {
        ESP_LOGE(SD_CARD_TAG, "SD fault");
        empty = true;
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

void sd_open_file(char *file, char *type)
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
    sprintf(filename, MOUNT_POINT "/%s_%d.txt", file, file_number);

    f = fopen(filename, type);
    if (f == NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "Failed to open file");
        return;
    }

    empty = true;
    ESP_LOGI(SD_CARD_TAG, "File opened");
}

void sd_close_file()
{
    if (f == NULL)
        return;

    if (!empty)
        nvs_write(FILE_NAME, nvs_read(FILE_NAME) + 1);

    fclose(f);
    f = NULL;
    ESP_LOGI(SD_CARD_TAG, "File closed");
}

void sd_write_data(uint8_t *data, size_t *len)
{
    if (f == NULL || !get_sd_det_state())
        return;

    if (empty)
        empty = false;

    xSemaphoreTake(sd_semaphore_handle, portMAX_DELAY);

    fwrite(data, sizeof(*data), *len, f);

    xSemaphoreGive(sd_semaphore_handle);
}

void sd_card_toggle()
{
    if (!sd_card_state())
    {
        ESP_LOGI(SD_CARD_TAG, "Start recording");
        sd_card_init(); // Mount SD card and create file to write
    }
    else
    {
        ESP_LOGI(SD_CARD_TAG, "Stop recording");
        sd_card_deinit(); // Close file and unmount SD card
    }

    spp_send_msg("s %d", sd_card_state());
}
