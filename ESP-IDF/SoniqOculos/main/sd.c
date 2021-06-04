#include "sd.h"

static sdmmc_card_t *card;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static FILE *f = NULL;

static bool sd_det_state = false;
static xTaskHandle s_sd_det_task_handle = NULL;
static void sd_det_task(void *arg);

bool sd_file_state()
{
    return f != NULL;
}

static void sd_det_task(void *arg)
{
    gpio_pad_select_gpio(SD_DET_PIN);                // Set GPIO
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT); // Set INPUT

    for (;;)
    {
        delay(SD_DET_DELAY);

        if (gpio_get_level(SD_DET_PIN) != sd_det_state)
        {
            sd_det_state = !sd_det_state;

            if (sd_det_state)
                ESP_LOGW(SD_CARD_TAG, "Card inserted");
            else // SD removed
            {
                ESP_LOGW(SD_CARD_TAG, "Card removed");

                if (f != NULL)
                    ESP_LOGE(SD_CARD_TAG, "Card fault");

                sd_card_deinit();
            }
        }
    }
    ////s_sd_det_task_handle = NULL;
    ////vTaskDelete(NULL);
}
void sd_det_task_init()
{
    if (SD_DEBUG)
        printf("SD task init\n");

    if (!s_sd_det_task_handle)
        xTaskCreate(sd_det_task, "sd_det_task", SD_DET_STACK_DEPTH, NULL, 11, &s_sd_det_task_handle);
}
/*void sd_det_task_deinit()
{
    if (SD_DEBUG)
        printf("SD task deinit\n");

    if (s_sd_det_task_handle)
    {
        vTaskDelete(s_sd_det_task_handle);
        s_sd_det_task_handle = NULL;
    }
}*/

void sd_card_init()
{
    if (f != NULL)
        return;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = host.slot;

    if (!sd_det_state)
    {
        ESP_LOGW(SD_CARD_TAG, "Insert card");
        return;
    }

    if (esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card) != ESP_OK)
    {
        ESP_LOGW(SD_CARD_TAG, "Reinsert card");
        return;
    }

    ESP_LOGI(SD_CARD_TAG, "Card mounted");

    sd_open_file("testing.txt", "wb"); //TODO Change name logic

    return;
}
void sd_card_deinit()
{
    if (f == NULL)
        return;

    sd_close_file();

    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);

    ESP_LOGI(SD_CARD_TAG, "Card unmounted");
    ESP_LOGW(SD_CARD_TAG, "Safe to remove card");
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

void sd_open_file(char *filename, char *type)
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
    if (f == NULL || !sd_det_state)
        return;

    fwrite(data, sizeof(*data), *len, f);
}
