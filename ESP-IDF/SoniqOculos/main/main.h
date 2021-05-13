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

#define I2S_DEBUG false

#define DEVICE_NAME "SoniqOculos"

#define MSG_BUFFER 128

#define LOGIC 3.3

#define HIGH 1
#define LOW 0

#define ON HIGH
#define OFF LOW

//* Modes
#define IDLE 0
#define MUSIC 1          // Speakers & Bone conductors
#define MUSIC_ISOLATED 2 // Bone conductors only
#define MUSIC_SPEAKERS 3 // Speakers only
#define PLAYBACK 4       // Microphones -> Bone conductors
#define RECORD 5         // Microphones -> SD Card

void delay(int millis);

void handleMsgs(char *msg);

void audioOnOff(bool state);

void process_data(uint8_t *data, size_t *len);