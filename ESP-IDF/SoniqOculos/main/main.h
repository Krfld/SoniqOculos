#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "nvs.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "freertos/queue.h"

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

//#include "esp_dsp.h"
#include "dsps_fir.h"

#define DEVICE_NAME "SoniqOculos"

#define GPIO_DEBUG OFF
#define I2S_DEBUG OFF
#define SD_DEBUG OFF
#define BT_DEBUG OFF

#define MAIN_TAG "MAIN"

#define LOGIC 3.3

#define HIGH 1
#define LOW 0

#define ON true
#define OFF false

#define MAX_VOLUME 100
#define VOLUME_INCREMENTS 10

#define PROCESSING OFF

//* Modes
#define MUSIC 0
#define RECORD_PLAYBACK 1

/**
 * @brief Delay in milliseconds
 * 
 * @param millis milliseconds to delay
 */
void delay(int millis);

void shutdown();

//* SPP Server
void server_welcome_msg(uint32_t handle);
void handleMsgs(char *msg);

//* Modes
void change_to_mode(int mode);

void music_toggle_devices();

void record_toggle_sd_card();
void record_toggle_bone_conductors();