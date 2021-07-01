#include "main.h"

#define B1 GPIO_NUM_13 // Button 1
#define B2 GPIO_NUM_12 // Button 2
#define B3 GPIO_NUM_14 // Button 3
#define B1_MASK 1 << 0 // 001 (1)
#define B2_MASK 1 << 1 // 010 (2)
#define B3_MASK 1 << 2 // 100 (4)

#define VIBRATE OFF
#define VIBRATOR_PIN GPIO_NUM_25
#define VIBRATION_DELAY 100

#define GPIO_STACK_DEPTH 3 * 1024
#define CHANGE_MODE_STACK_DEPTH 2 * 1024
#define RELEASING_STACK_DEPTH 1 * 1024
#define POWER_OFF_STACK_DEPTH 2 * 1024
#define VOLUME_STACK_DEPTH 2 * 1024

#define DEBOUNCE 50              // ms
#define RELEASE_DELAY 250        // ms
#define POWER_OFF_HOLD_TIME 2000 // ms
#define VOLUME_CHANGE_PERIOD 250 // ms
#define COMMAND_DELAY 500        // ms

int get_mode();

bool get_sd_det_state();

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
