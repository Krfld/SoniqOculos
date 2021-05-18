#include "main.h"

#include "bt_app_core.h"
#include "bt_app_av.h"

#include "i2s.h"

#define BT_STACK_DEPTH 2 * 1024

#define BT_SPP_TAG "BT_SPP"

void bt_init();

void bt_music_init();
void bt_music_deinit();

void bt_send_cmd(uint8_t command);
