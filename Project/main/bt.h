#include "main.h"

#include "bt_app_core.h"
#include "bt_app_av.h"

#include "i2s.h"

#define BT_SPP_TAG "BT_SPP"

void bt_init();

bool bt_is_music_active();

void bt_music_init();
void bt_music_deinit();