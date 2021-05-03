#include "main.h"

#define DEBOUNCE 50 // ms

#define BUTTON_START 13
#define BUTTON_VOLUME_UP 12
#define BUTTON_VOLUME_DOWN 14

#define SD_CARD_DET_DELAY 2000 // ms

void wait_for_sd_card();

void speakers_pin_reset();
void microphones_pin_reset();

void gpio_task_start_up(void);
void gpio_task_shut_down(void);
