#include "main.h"

#include "bt_app_core.h"
#include "bt_app_av.h"

#include "i2s.h"

#define BT_SPP_TAG "BT_SPP"

#define RINGBUF_SIZE 3 * 4096
#define BT_I2S_STACK_DEPTH 2 * 1024

void bt_init();

void bt_music_init();
void bt_music_deinit();

void bt_send_cmd(uint8_t command);
