#include "gpio.h"

#include "i2s.h"
#include "sd.h"

static xTaskHandle s_gpio_task_handle = NULL;
static void gpio_task_handler(void *arg);

void wait_for_sd_card()
{
    gpio_pad_select_gpio(SD_DET_PIN);
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT);

    do //* Make sure the card is inserted
    {
        while (!gpio_get_level(SD_DET_PIN))
            ;
        delay(SD_CARD_DET_DELAY);
    } while (!gpio_get_level(SD_DET_PIN));
}

void i2s_pins_reset(int ws_pin, int bck_pin, int data_pin)
{
    gpio_pad_select_gpio(ws_pin);                 // Set GPIO
    gpio_set_direction(ws_pin, GPIO_MODE_OUTPUT); // Set OUTPUT
    gpio_set_level(ws_pin, LOW);                  // Set LOW

    gpio_pad_select_gpio(bck_pin);                 // Set GPIO
    gpio_set_direction(bck_pin, GPIO_MODE_OUTPUT); // Set OUTPUT
    gpio_set_level(bck_pin, LOW);

    gpio_pad_select_gpio(data_pin);                 // Set GPIO
    gpio_set_direction(data_pin, GPIO_MODE_OUTPUT); // Set OUTPUT
    gpio_set_level(data_pin, LOW);                  // Set LOW
}

static void gpio_task_handler(void *arg)
{
    bool buttons_state[3] = {false, false, false};
    bool sd_det_state = false;

    gpio_pad_select_gpio(BUTTON_1);                   // Set GPIO
    gpio_set_direction(BUTTON_1, GPIO_MODE_INPUT);    // Set INPUT
    gpio_set_pull_mode(BUTTON_1, GPIO_PULLDOWN_ONLY); // Set PULLDOWN

    gpio_pad_select_gpio(BUTTON_2);                   // Set GPIO
    gpio_set_direction(BUTTON_2, GPIO_MODE_INPUT);    // Set INPUT
    gpio_set_pull_mode(BUTTON_2, GPIO_PULLDOWN_ONLY); // Set PULLDOWN

    gpio_pad_select_gpio(BUTTON_3);                   // Set GPIO
    gpio_set_direction(BUTTON_3, GPIO_MODE_INPUT);    // Set INPUT
    gpio_set_pull_mode(BUTTON_3, GPIO_PULLDOWN_ONLY); // Set PULLDOWN

    gpio_pad_select_gpio(SD_DET_PIN);                // Set GPIO
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT); // Set INPUT

    for (;;) //TODO Handle button combos
    {
        delay(DEBOUNCE); // DEBOUNCE

        if (gpio_get_level(BUTTON_1) != buttons_state[0])
        {
            buttons_state[0] = !buttons_state[0];
            if (buttons_state[0])
            {
                printf("Clicked BUTTON 1\n");
            }
            else
            {
                printf("Released BUTTON 1\n");
            }
        }

        if (gpio_get_level(BUTTON_2) != buttons_state[1])
        {
            buttons_state[1] = !buttons_state[1];
            if (buttons_state[1])
            {
                printf("Clicked BUTTON 2\n");
            }
            else
            {
                printf("Released BUTTON 2\n");
            }
        }

        if (gpio_get_level(BUTTON_3) != buttons_state[2])
        {
            buttons_state[2] = !buttons_state[2];
            if (buttons_state[2])
            {
                printf("Clicked BUTTON 3\n");
            }
            else
            {
                printf("Released BUTTON 3\n");
            }
        }

        /*if (gpio_get_level(SD_DET_PIN) != sd_det_state)
        {
            sd_det_state = !sd_det_state;
            if (sd_det_state)
            {
                printf("Inserting card...\n");
                delay(SD_CARD_DET_DELAY);
                if (gpio_get_level(SD_DET_PIN))
                {
                    printf("Card inserted\n");
                    sd_init();
                    sd_open_file("samples.txt", "wb");
                }
            }
            else
            {
                printf("Removing card...\n");
                sd_deinit();
                printf("Card removed\n");
            }
        }*/
    }
}

void gpio_init()
{
    xTaskCreate(gpio_task_handler, "gpio_task", GPIO_STACK_DEPTH, NULL, 10, &s_gpio_task_handle);
}
void gpio_deinit()
{
    if (s_gpio_task_handle)
    {
        vTaskDelete(s_gpio_task_handle);
        s_gpio_task_handle = NULL;
    }
}
