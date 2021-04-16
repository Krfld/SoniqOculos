#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "bt_app_core.h"
#include "bt_app_av.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "driver/i2s.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_gatt_common_api.h"

#define BT_BLE_COEX_TAG "SoniqOculos"
#define BT_DEVICE_NAME "SoniqOculos"
#define BLE_ADV_NAME "[CONFIG] SoniqOculos"

#define I2S_BCK_PIN(i2s_num) (i2s_num ? 26 : 26)
#define I2S_WS_PIN(i2s_num) (i2s_num ? 25 : 25)
#define I2S_DATA_PIN(i2s_num) (i2s_num ? 22 : 22)

void delay(int millis);

void i2s_setup();

void tools();