#include "main.h"

#define B1 GPIO_NUM_13 // Button 1
#define B2 GPIO_NUM_12 // Button 2
#define B3 GPIO_NUM_14 // Button 3
#define B1_MASK 1 << 0 // 001 (1)
#define B2_MASK 1 << 1 // 010 (2)
#define B3_MASK 1 << 2 // 100 (4)

#define GPIO_STACK_DEPTH 2 * 1024
#define RELEASING_STACK_DEPTH 2 * 1024
#define POWER_OFF_STACK_DEPTH 2 * 1024
#define VOLUME_STACK_DEPTH 2 * 1024

#define DEBOUNCE 50                   // ms
#define RELEASE_DELAY 250             // ms
#define POWER_OFF_HOLD_TIME 2000      // ms
#define VOLUME_CHANGE_START_DELAY 500 // ms
#define VOLUME_CHANGE_PERIOD 500      // ms
#define COMMAND_DELAY 500             // ms

#define SD_CARD_DET_DELAY 2000 // ms

void wait_for_sd_card();

void i2s_pins_reset(int ws_pin, int bck_pin, int data_pin);

/**
 * @brief Create gpio task
 * 
 */
void gpio_task_init();

/**
 * @brief Delete gpio task
 * 
 */
void gpio_task_deinit();
