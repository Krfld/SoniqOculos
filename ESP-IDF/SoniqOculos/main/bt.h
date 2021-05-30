#include "main.h"

#include "bt_app_core.h"
#include "bt_app_av.h"

#include "i2s.h"

#define BT_SPP_TAG "BT_SPP"

#define RINGBUF_SIZE 3 * 4096
#define BT_I2S_STACK_DEPTH 3 * 1024 // 2* might be enough

void set_bda(uint8_t *addr);

void bt_init();

void bt_music_init();
void bt_music_deinit();

void bt_send_avrc_cmd(uint8_t command);
