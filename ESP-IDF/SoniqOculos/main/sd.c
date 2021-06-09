#include "sd.h"

#include "gpio.h"
#include "nvs_tools.h"

static sdmmc_card_t *card;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static FILE *f = NULL;

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
}
/*void spi_deinit()
{
    spi_bus_free(host.slot);
}*/

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

    return;
}
void sd_card_deinit()
{
    sd_close_file();

    if (card == NULL)
        return;

    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    card = NULL;

    ESP_LOGI(SD_CARD_TAG, "Card unmounted");
    ESP_LOGW(SD_CARD_TAG, "Safe to remove card");
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

    char name[64];
    int32_t file_number = nvs_read(FILE_NAME);
    sprintf(name, MOUNT_POINT "/%s_%d.txt", file, file_number);

    // Check if previous file is empty
    f = fopen(name, type);
    if (f != NULL)
        if (fseek(f, 0, SEEK_END) == 0)
            if (ftell(f) != 0) // TODO Check performance with large files
            {
                file_number++;
                fclose(f);
                f == NULL;
            }

    if (f == NULL) // File is already opened if empty
    {
        sprintf(name, MOUNT_POINT "/%s_%d.txt", file, file_number);
        f = fopen(name, type);

        if (f == NULL)
        {
            ESP_LOGE(SD_CARD_TAG, "Failed to open file");
            return;
        }
    }

    nvs_write(file_number, FILE_NAME);
    ESP_LOGI(SD_CARD_TAG, "File opened");
}

void sd_close_file()
{
    if (f == NULL)
        return;

    fclose(f);
    f = NULL;
    ESP_LOGI(SD_CARD_TAG, "File closed");
}

void sd_write_data(uint8_t *data, size_t *len)
{
    if (f == NULL || !get_sd_det_state())
        return;

    fwrite(data, sizeof(*data), *len, f);
}
