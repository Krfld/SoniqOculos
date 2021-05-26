#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "nvs.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"

#include "esp_log.h"
#include "esp_vfs_fat.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_spp_api.h"

#include "driver/i2s.h"
#include "driver/dac.h"
#include "driver/gpio.h"

//#include "esp_dsp.h"
#include "dsps_fir.h"

#define GPIO_DEBUG OFF
#define I2S_DEBUG OFF
#define SD_DEBUG OFF
#define BT_DEBUG OFF

#define DEVICE_NAME "SoniqOculos"

#define MAIN_TAG "MAIN"

#define LOGIC 3.3

#define HIGH 1
#define LOW 0

#define ON true
#define OFF false

//* Modes
#define MUSIC 0
#define RECORD_PLAYBACK 1

#define MSG_BUFFER_SIZE 128

/**
 * @brief Delay in milliseconds
 * 
 * @param millis milliseconds to delay
 */
void delay(int millis);

/**
 * @brief Handle messages recevied
 * 
 * @param msg message received
 */
void handleMsgs(char *msg);

/**
 * @brief Process data
 * 
 * @param data buffer of samples to process
 * @param len buffer length
 */
void process_data(uint8_t *data, size_t *len);
