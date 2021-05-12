/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include <math.h>

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
//#include "sdkconfig.h"

//#include "driver/sdmmc_host.h"

#define MICROPHONES_I2S_NUM I2S_NUM_0
#define MICROPHONES_WS_PIN 32
#define MICROPHONES_BCK_PIN 27
#define MICROPHONES_DATA_PIN 33

#define BONE_CONDUCTORS_I2S_NUM I2S_NUM_1
#define BONE_CONDUCTORS_WS_PIN 3
#define BONE_CONDUCTORS_BCK_PIN 21
#define BONE_CONDUCTORS_DATA_PIN 19

#define DMA_BUFFER_COUNT 8
#define DMA_BUFFER_LEN 1024

//* Bone conductors pin configuration
static i2s_pin_config_t bone_conductors_pin_config = {
    .ws_io_num = BONE_CONDUCTORS_WS_PIN,
    .bck_io_num = BONE_CONDUCTORS_BCK_PIN,
    .data_out_num = BONE_CONDUCTORS_DATA_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE};

//* Microphones pin configuration
static i2s_pin_config_t microphones_pin_config = {
    .ws_io_num = MICROPHONES_WS_PIN,
    .bck_io_num = MICROPHONES_BCK_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = MICROPHONES_DATA_PIN};

//* Output configuration
static i2s_config_t i2s_config_tx = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX,
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

static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"

// DMA channel to be used by the SPI peripheral
#ifndef SPI_DMA_CHAN
#define SPI_DMA_CHAN 1
#endif //SPI_DMA_CHAN

#define PIN_NUM_CLK 16
#define PIN_NUM_MISO 4
#define PIN_NUM_MOSI 2
#define PIN_NUM_CS 15

#define PIN_NUM_DET 36

void app_main(void)
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    gpio_pad_select_gpio(PIN_NUM_DET);
    gpio_set_direction(PIN_NUM_DET, GPIO_MODE_INPUT);

    do
    {
        while (!gpio_get_level(PIN_NUM_DET))
            ;
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    } while (!gpio_get_level(PIN_NUM_DET));

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                          "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Bone conductors setup
    if (i2s_driver_install(BONE_CONDUCTORS_I2S_NUM, &i2s_config_tx, 0, NULL) != ESP_OK)
        printf("Bone conductors i2s driver install failed\n");

    if (i2s_set_pin(BONE_CONDUCTORS_I2S_NUM, &bone_conductors_pin_config) != ESP_OK)
        printf("Bone conductors i2s set pin failed\n");

    // Microphones setup
    if (i2s_driver_install(MICROPHONES_I2S_NUM, &i2s_config_rx, 0, NULL) != ESP_OK)
        printf("Microphones i2s driver install failed\n");

    if (i2s_set_pin(MICROPHONES_I2S_NUM, &microphones_pin_config) != ESP_OK)
        printf("Microphones i2s set pin failed\n");

    size_t bytes_read = 0, bytes_written = 0;
    uint8_t data[DMA_BUFFER_LEN] = {0};

    ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen(MOUNT_POINT "/samples.txt", "wb");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    i2s_zero_dma_buffer(MICROPHONES_I2S_NUM);
    i2s_zero_dma_buffer(BONE_CONDUCTORS_I2S_NUM);

    bool det_state = false;

    for (;;)
    {
        i2s_read(MICROPHONES_I2S_NUM, data, sizeof(data), &bytes_read, portMAX_DELAY);

        //fwrite(data, sizeof(uint8_t), bytes_read, f);

        /*for (size_t i = 0; i < bytes_read; i += 4)
        {
            printf("New sample\n");
            printf("%d ", data[i]);
            printf("%d ", data[i + 1]);
            printf("%d ", data[i + 2]);
            printf("%d\n", data[i + 3]);
            printf("%d\n", (short)((data[i + 3] << 8) | (data[i + 2])));
            printf("%d\n", ((data[i + 3] << 8) | (data[i + 2])));*
            fprintf(f, "%d\n", (short)((data[i + 3] << 8) | (data[i + 2])));
        }

        if (esp_timer_get_time() / 1000 / 1000 == 60)
            break;*/

        //TODO Test with pin 36
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (gpio_get_level(PIN_NUM_DET) != det_state)
        {
            det_state = !det_state;
            if (det_state)
                printf("Inserting card...\n");
            else

                printf("Removing card...\n");
        }

        //i2s_write(BONE_CONDUCTORS_I2S_NUM, data, bytes_read, &bytes_written, portMAX_DELAY);
    }

    //fclose(f);
    ESP_LOGI(TAG, "File written");

    /*
    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(TAG, "Opening file");
    FILE *f = fopen(MOUNT_POINT "/hello.txt", "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(MOUNT_POINT "/foo.txt", &st) == 0)
    {
        // Delete it if it exists
        unlink(MOUNT_POINT "/foo.txt");
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file");
    if (rename(MOUNT_POINT "/hello.txt", MOUNT_POINT "/foo.txt") != 0)
    {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file");
    f = fopen(MOUNT_POINT "/foo.txt", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
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
    ESP_LOGI(TAG, "Read from file: '%s'", line);
    */

    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");
    spi_bus_free(host.slot);
}
