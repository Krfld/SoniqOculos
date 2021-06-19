#include "main.h"

#include "bt_app_core.h"
#include "bt_app_av.h"

#include "i2s.h"
#include "sd.h"
#include "dsp.h"

#define MSG_BUFFER_SIZE 128

#define BT_I2S_STACK_DEPTH 4 * 1024
#define RINGBUFFER_SIZE 3 * DATA_LENGTH

void save_last_device(uint8_t *addr);

void spp_send_msg(char *msg);

void bt_init();

void bt_music_init();
void bt_music_deinit();
