#include "sd.h"

#include "gpio.h"

static sdmmc_card_t *card;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static FILE *f = NULL;

void sd_init()
{
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
    ret = spi_bus_initialize(host.slot, &bus_cfg, 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SD_CARD_TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = host.slot;

    wait_for_sd_card();

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
        return;
    }

    ESP_LOGI(SD_CARD_TAG, "Card mounted");

    /*
    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(SD_CARD_TAG, "Opening file");
    FILE *f = fopen(MOUNT_POINT "/hello.txt", "w");
    if (f == NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(SD_CARD_TAG, "File written");

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(MOUNT_POINT "/foo.txt", &st) == 0)
    {
        // Delete it if it exists
        unlink(MOUNT_POINT "/foo.txt");
    }

    // Rename original file
    ESP_LOGI(SD_CARD_TAG, "Renaming file");
    if (rename(MOUNT_POINT "/hello.txt", MOUNT_POINT "/foo.txt") != 0)
    {
        ESP_LOGE(SD_CARD_TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(SD_CARD_TAG, "Reading file");
    f = fopen(MOUNT_POINT "/foo.txt", "r");
    if (f == NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char *pos = strchr(line, '\n');
    if (pos)
    {
        *pos = '\0';
    }
    ESP_LOGI(SD_CARD_TAG, "Read from file: '%s'", line);
    */
}

void sd_deinit()
{
    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    ESP_LOGI(SD_CARD_TAG, "Card unmounted");
    spi_bus_free(host.slot);
}

void open_file(char *filename, char *type)
{
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

void close_file()
{
    fclose(f);
    f = NULL;
    ESP_LOGI(SD_CARD_TAG, "File closed");
}

void write_sample(uint8_t *data, size_t *len)
{
    if (f == NULL)
    {
        ESP_LOGE(SD_CARD_TAG, "Failed to write to file");
        return;
    }

    //fwrite(data, 1, *len, f);
    //fputs((char *)data, f);
    //fprintf(f, "%d\n", sample);
}
