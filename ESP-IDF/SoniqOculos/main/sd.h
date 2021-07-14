#include "main.h"

#include "nvs_tools.h"

#define MOUNT_POINT "/sdcard"
#define FILE_NAME "rec"

#define WRITE "w"
#define READ "r"

#define SD_CLK_PIN GPIO_NUM_16 // RX2
#define SD_MISO_PIN GPIO_NUM_4
#define SD_MOSI_PIN GPIO_NUM_2
#define SD_CS_PIN GPIO_NUM_15

#define SD_DET_PIN GPIO_NUM_35

#define SD_DET_DELAY 50 // ms

#define SD_DET_STACK_DEPTH 3 * 1024

//* WAV File

#define FMT_SIZE 16
#define AUDIO_FORMAT 1 // PCM
#define NUM_CHANNELS 2
#define SAMPLE_RATE 44100

#define BYTE_RATE (SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8)
#define BLOCK_ALIGN (NUM_CHANNELS * BITS_PER_SAMPLE / 8)

#define BITS_PER_SAMPLE 16

/**
 * @brief See if card is mounted and file opened
 * 
 * @return true if SD is ready, false otherwise
 */
bool sd_card_state();

void sd_card_init();
void sd_card_deinit();

/**
 * @brief Setup SPI interface
 * 
 */
void spi_init();

/**
 * @brief Open a file to write
 * 
 * @param file name of the file
 */
void sd_open_file(char *file);
/**
 * @brief Close the file
 * 
 */
void sd_close_file();

/**
 * @brief Write data to SD card
 * 
 * @param data buffer of samples to write
 * @param len buffer length
 */
void sd_write_data(uint8_t *data, size_t *len);

/**
 * @brief Open or close file
 * 
 * @param state true opens the file, false closes it
 */
void sd_set_card(bool state);
