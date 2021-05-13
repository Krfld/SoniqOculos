#include "sd.h"

#include "gpio.h"

static sdmmc_card_t *card;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static FILE *f = NULL;

static bool sd_mounted = false;
bool sd_is_card_mounted()
{
    return sd_mounted;
}
void sd_init()
{
    if (sd_is_card_mounted())
        return;

    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    ESP_LOGI(SD_CARD_TAG, "Initializing SD card");

    ESP_LOGI(SD_CARD_TAG, "Using SPI peripheral");

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_MOSI_PIN,
        .miso_io_num = SD_MISO_PIN,
        .sclk_io_num = SD_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };
    /*ret = */ spi_bus_initialize(host.slot, &bus_cfg, 1);
    /*if (ret != ESP_OK)
    {
        ESP_LOGE(SD_CARD_TAG, "Failed to initialize bus.");
        spi_bus_free(host.slot);
        return;
    }*/

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(SD_CARD_TAG, "Failed to mount filesystem. "
                                  "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            ESP_LOGE(SD_CARD_TAG, "Failed to initialize the card (%s). "
                                  "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        spi_bus_free(host.slot);
        return;
    }

    sd_mounted = true;
    ESP_LOGI(SD_CARD_TAG, "Card mounted");

    //sd_open_file("samples.txt", "wb");
}
void sd_deinit()
{
    if (!sd_is_card_mounted())
        return;

    sd_close_file();

    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    sd_mounted = false;
    ESP_LOGI(SD_CARD_TAG, "Card unmounted");
    spi_bus_free(host.slot);
}

void sd_open_file(char *filename, char *type)
{
    if (!sd_is_card_mounted())
    {
        ESP_LOGE(SD_CARD_TAG, "Card is not mounted");
        return;
    }

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
    if (!sd_is_card_mounted())
    {
        ESP_LOGE(SD_CARD_TAG, "Card is not mounted");
        return;
    }

    if (f == NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "Failed to write to file");
        return;
    }

    fwrite(data, sizeof(uint8_t), *len, f);
}
