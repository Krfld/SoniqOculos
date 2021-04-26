#include "gpio.h"

void gpio_set(gpio_num_t pin, size_t level)
{
    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, level);
}