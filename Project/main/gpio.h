#include "main.h"

#define DEBOUNCE 100 // ms

#define BUTTON_1 13
#define BUTTON_2 12
#define BUTTON_3 14

#define SD_CARD_DET_DELAY 2000 // ms

#define GPIO_STACK_DEPTH 3 * 1024

void wait_for_sd_card();

void i2s_pins_reset(int ws_pin, int bck_pin, int data_pin);

void gpio_init();
void gpio_deinit();
