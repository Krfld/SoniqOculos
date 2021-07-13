#include "main.h"

#define NONE 0
#define SPEAKERS 1
#define MICROPHONES 2

#define BOTH_DEVICES 0
#define ONLY_BONE_CONDUCTORS 1
#define ONLY_SPEAKERS 2

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

/**
 * @brief Get active devices
 * 
 * @return int BOTH_DEVICES or ONLY_BONE_CONDUCTORS or ONLY_SPEAKERS
 */
int get_devices();

/**
 * @brief Get the state of the specified device
 * 
 * @param device SPEAKERS_MICROPHONES_I2S_NUM or BONE_CONDUCTORS_I2S_NUM
 * @return true if ON, false if OFF
 */
bool i2s_get_device_state(int device);
/**
 * @brief Set the state of the specified device
 * 
 * @param device SPEAKERS_MICROPHONES_I2S_NUM or BONE_CONDUCTORS_I2S_NUM
 * @param state ON of OFF
 */
void i2s_set_device_state(int device, bool state);

/**
 * @brief Change to a specific device
 * 
 * @param dev BOTH_DEVICES or ONLY_BONE_CONDUCTORS or ONLY_SPEAKERS
 */
void i2s_change_to_devices(int dev);
/**
 * @brief Toggle between devices
 * 
 */
void i2s_toggle_devices();

/**
 * @brief Set BCD state
 * 
 * @param state ON or OFF
 */
void i2s_set_bone_conductors(bool state);

/**
 * @brief Turn devices ON
 * 
 */
void i2s_turn_devices_on();
/**
 * @brief Turn devices OFF
 * 
 */
void i2s_turn_devices_off();

/**
 * @brief Setup speakers interface
 * 
 */
void speakers_init();

/**
 * @brief Setup microphones interface
 * 
 */
void microphones_init();

/**
 * @brief Setup BCD interface
 * 
 */
void bone_conductors_init();

/**
 * @brief Setup i2s components
 * 
 */
void i2s_init();

/**
 * @brief Write data to I2S interface
 * 
 * @param data buffer of samples to write
 * @param len buffer length
 */
void i2s_write_data(uint8_t *data, size_t *len);
