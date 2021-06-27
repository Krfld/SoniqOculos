#include "main.h"

//* Devices
#define NONE 0
#define SPEAKERS 1
#define MICROPHONES 2
//#define BONE_CONDUCTORS 3

#define SPEAKERS_MICROPHONES_I2S_NUM I2S_NUM_0
#define BONE_CONDUCTORS_I2S_NUM I2S_NUM_1

#define SPEAKERS_SD_PIN GPIO_NUM_22
#define BONE_CONDUCTORS_SD_PIN GPIO_NUM_23

#define SPEAKERS_WS_PIN GPIO_NUM_18
#define SPEAKERS_BCK_PIN GPIO_NUM_5
#define SPEAKERS_DATA_PIN GPIO_NUM_17 // TX2

#define MICROPHONES_WS_PIN GPIO_NUM_32
#define MICROPHONES_BCK_PIN GPIO_NUM_27
#define MICROPHONES_DATA_PIN GPIO_NUM_33

#define BONE_CONDUCTORS_WS_PIN GPIO_NUM_3 // RX0
#define BONE_CONDUCTORS_BCK_PIN GPIO_NUM_21
#define BONE_CONDUCTORS_DATA_PIN GPIO_NUM_19

#define I2S_DMA_BUFFER_COUNT 8
#define I2S_DMA_BUFFER_LEN 1024

#define I2S_READ_STACK_DEPTH 7 * 1024

#define READ_TASK_IDLE_DELAY 500 // ms
#define SINCRONIZE_DELAY 300     // ms
//#define DEVICE_DEINIT_DELAY 100  // ms

bool i2s_get_device_state(int device);
void i2s_set_device_state(int device, bool state);

void i2s_change_to_devices(int dev);
void i2s_toggle_devices();

void i2s_toggle_bone_conductors();

void i2s_turn_devices_on();
void i2s_turn_devices_off();

void speakers_init();

void microphones_init();

void bone_conductors_init();
//void bone_conductors_deinit();

void i2s_init();

/**
 * @brief Write data to I2S interface
 * 
 * @param data buffer of samples to write
 * @param len buffer length
 */
void i2s_write_data(uint8_t *data, size_t *len);
