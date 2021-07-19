#include "main.h"

#include "bt_app_core.h"
#include "bt_app_av.h"

#include "gpio.h"
#include "i2s.h"
#include "sd.h"
#include "dsp.h"

#define MSG_BUFFER_SIZE 128

#define BT_I2S_STACK_DEPTH 4 * 1024
#define RINGBUFFER_SIZE 3 * DATA_LENGTH

/**
 * @brief Stores device address
 * 
 * @param addr address of the device
 */
void save_last_device(uint8_t *addr);

/**
 * @brief Send SPP message
 * 
 * @param msg message to send
 * @param ... parameters of the message
 */
void spp_send_msg(char *msg, ...);

/**
 * @brief Init bluetooth peripherals
 * 
 */
void bt_init();

/**
 * @brief Init bluetooth music components
 * 
 */
void bt_music_init();
/**
 * @brief Deinit bluetooth music components
 * 
 */
void bt_music_deinit();
