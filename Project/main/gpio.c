#include "gpio.h"

void gpio_direction(gpio_num_t pin)
{
    gpio_pad_select_gpio(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}
