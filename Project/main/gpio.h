#include "main.h"

#define DEBOUNCE 50 // ms

#define BUTTON_START 13
#define BUTTON_VOLUME_UP 12
#define BUTTON_VOLUME_DOWN 14

#define SD_CARD_DET_DELAY 2000 // ms

void wait_for_sd_card();

void i2s_pins_reset(int ws_pin, int bck_pin, int data_pin);

void gpio_init();
void gpio_deinit();
