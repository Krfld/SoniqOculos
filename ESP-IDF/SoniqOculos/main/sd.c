#include "sd.h"

#include "gpio.h"

static sdmmc_card_t *card;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static FILE *f = NULL;

static bool sd_card_mounted = false;

//TODO Create task to detect card

bool sd_init()
{
    if (sd_card_mounted)
        return false;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = host.slot;

    gpio_pad_select_gpio(SD_DET_PIN);                // Set GPIO
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT); // Set INPUT

    if (!gpio_get_level(SD_DET_PIN))
    {
        ESP_LOGW(SD_CARD_TAG, "Insert card");
        return false;
    }

    if (esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card) != ESP_OK)
    {
        ESP_LOGW(SD_CARD_TAG, "Reinsert card");
        return false;
    }

    sd_card_mounted = true;

    ESP_LOGI(SD_CARD_TAG, "Card mounted");

    return true;
    //sd_open_file("samples.txt", "wb");
}
void sd_deinit()
{
    if (!sd_card_mounted)
        return;

    sd_card_mounted = false;

    sd_close_file();

    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);

    ESP_LOGI(SD_CARD_TAG, "Card unmounted");
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
void spi_deinit()
{
    spi_bus_free(host.slot);
}

void sd_open_file(char *filename, char *type)
{
    /*if (!sd_is_card_mounted())
    {
        ESP_LOGE(SD_CARD_TAG, "Card is not mounted");
        return;
    }*/

    if (f != NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "File already opened");
        return;
    }

    char file[64];
    sprintf(file, MOUNT_POINT "/%s", filename);

    f = fopen(file, type);
    if (f == NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "Failed to open file");
        return;
    }

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
    //TODO Test writing when no card inserted

    /*if (!sd_is_card_mounted())
    {
        ESP_LOGE(SD_CARD_TAG, "Card is not mounted");
        return;
    }*/

    if (f == NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "No file to write");
        return;
    }

    fwrite(data, sizeof(*data), *len, f);
}
