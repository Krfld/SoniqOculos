#include "main.h"

#define DEBOUNCE 50 // ms

#define BUTTON_1 GPIO_NUM_13
#define BUTTON_2 GPIO_NUM_12
#define BUTTON_3 GPIO_NUM_14
#define BUTTON_1_MASK 1 << 0 // 0b001 | 1
#define BUTTON_2_MASK 1 << 1 // 0b010 | 2
#define BUTTON_3_MASK 1 << 2 // 0b100 | 4

#define SD_CARD_DET_DELAY 2000 // ms

#define GPIO_STACK_DEPTH 2 * 1024

void wait_for_sd_card();

void i2s_pins_reset(int ws_pin, int bck_pin, int data_pin);

/**
 * @brief Create gpio task
 * 
 */
void gpio_init();

/**
 * @brief Delete gpio task
 * 
 */
void gpio_deinit();
