#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "math.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "esp_sleep.h"

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
#include "driver/rtc_io.h"

#include "esp_dsp.h"

#define DEVICE_NAME "SoniqOculos"

#define GPIO_DEBUG OFF
#define I2S_DEBUG OFF
#define SD_DEBUG OFF
#define BT_DEBUG OFF
#define DSP_DEBUG OFF

#define LOGIC 3.3
#define INT16 32768.0 // 2^15

#define HIGH 1
#define LOW 0

#define ON true
#define OFF false

#define MUSIC 0
#define RECORD 1

/**
 * @brief Delay in milliseconds
 * 
 * @param millis milliseconds to delay
 */
void delay(int millis);

/**
 * @brief Get mode
 * 
 * @return MUSIC if in music mode, RECORD if in record mode
 */
int get_mode();

/**
 * @brief Enter deep-sleep mode
 * 
 */
void shutdown();

/**
 * @brief Handle messages received from the app
 * 
 * @param msg message received
 */
void handleMsgs(char *msg);

/**
 * @brief Change to a specific mode
 * 
 * @param mode MUSIC to change to music mode, RECORD to change to record mode
 */
void change_to_mode(int mode);
