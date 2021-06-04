#include "main.h"

#include "bt_app_core.h"
#include "bt_app_av.h"

#include "i2s.h"
#include "dsp.h"

#define BT_TAG "BT"
#define BT_SPP_TAG "BT_SPP"

#define MSG_BUFFER_SIZE 128

#define RINGBUFFER_SIZE 3 * DATA_LENGTH
#define BT_I2S_STACK_DEPTH 2 * 1024 //? 2k is enough?

void save_last_device(uint8_t *addr);

void bt_init();

void bt_music_init();
void bt_music_deinit();
