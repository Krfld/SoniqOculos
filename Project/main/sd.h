#include "main.h"

#define SD_CARD_TAG "SD_CARD"

#define MOUNT_POINT "/sdcard"

#define SD_MISO_PIN 2
#define SD_MOSI_PIN 4
#define SD_CLK_PIN 15
#define SD_CS_PIN 16

void sd_init();

void open_file(char *filename, char *type);
void close_file();

void write_data(uint8_t *data, size_t len);