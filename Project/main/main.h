#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "sys/lock.h"
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "freertos/xtensa_api.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

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

#define DEVICE_NAME "SoniqOculos"

#define LED_BUILTIN 2

#define MSG_BUFFER 128

#define VDD 3.3

#define HIGH 1
#define LOW 0

#define ON 1
#define OFF 0

#define RIGHT_CHANEL DAC_CHANNEL_1
#define LEFT_CHANEL DAC_CHANNEL_2

void process_data(uint8_t *data, size_t len);