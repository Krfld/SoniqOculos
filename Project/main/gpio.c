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
    gpio_pad_select_gpio(ws_pin);
    gpio_set_direction(ws_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(ws_pin, LOW);

    gpio_pad_select_gpio(bck_pin);
    gpio_set_direction(bck_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(bck_pin, LOW);

    gpio_pad_select_gpio(data_pin);
    gpio_set_direction(data_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(data_pin, LOW);
}

static void gpio_task_handler(void *arg)
{
    bool button_1_state = false, button_2_state = false, button_3_state = false;
    bool sd_det_state = false;

    gpio_pad_select_gpio(BUTTON_1);
    gpio_set_direction(BUTTON_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_1, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(BUTTON_2);
    gpio_set_direction(BUTTON_2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_2, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(BUTTON_3);
    gpio_set_direction(BUTTON_3, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_3, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(SD_DET_PIN);
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT);

    for (;;) //TODO Handle button combos
    {
        delay(DEBOUNCE);

        if (gpio_get_level(BUTTON_1) != button_1_state)
        {
            button_1_state = !button_1_state;
            if (button_1_state)
            {
                printf("Clicked BUTTON 1\n");
            }
            else
            {
                printf("Released BUTTON 1\n");
                set_mode(MUSIC);
            }
        }

        if (gpio_get_level(BUTTON_2) != button_2_state)
        {
            button_2_state = !button_2_state;
            if (button_2_state)
            {
                printf("Clicked BUTTON 2\n");
            }
            else
            {
                printf("Released BUTTON 2\n");
                set_mode(MUSIC_SPEAKERS);
            }
        }

        if (gpio_get_level(BUTTON_3) != button_3_state)
        {
            button_3_state = !button_3_state;
            if (button_3_state)
            {
                printf("Clicked BUTTON 3\n");
            }
            else
            {
                printf("Released BUTTON 3\n");
                set_mode(PLAYBACK);
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
