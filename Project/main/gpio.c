#include "gpio.h"

#include "i2s.h"
#include "sd.h"

static xTaskHandle s_gpio_task_handle = NULL;
static void gpio_task_handler(void *arg);

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

static void gpio_task_handler(void *arg)
{
    bool button_start_state = false, button_volume_up_state = false, button_volume_down_state = false;
    bool sd_det_state = false;

    gpio_pad_select_gpio(BUTTON_START);
    gpio_set_direction(BUTTON_START, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_START, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(BUTTON_VOLUME_UP);
    gpio_set_direction(BUTTON_VOLUME_UP, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_VOLUME_UP, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(BUTTON_VOLUME_DOWN);
    gpio_set_direction(BUTTON_VOLUME_DOWN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_VOLUME_DOWN, GPIO_PULLDOWN_ONLY);

    gpio_pad_select_gpio(SD_DET_PIN);
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT);

    for (;;)
    {
        delay(DEBOUNCE);

        if (gpio_get_level(BUTTON_START) != button_start_state)
        {
            button_start_state = !button_start_state;
            if (button_start_state)
            {
                printf("Clicked START\n");
                if (!sd_is_card_mounted())
                    sd_init();
                else
                    sd_deinit();
            }
            else
                printf("Released START\n");
        }

        if (gpio_get_level(BUTTON_VOLUME_UP) != button_volume_up_state)
        {
            button_volume_up_state = !button_volume_up_state;
            if (button_volume_up_state)
            {
                printf("Clicked VOLUME UP\n");
                set_mode(MUSIC);
            }
            else
                printf("Released VOLUME UP\n");
        }

        if (gpio_get_level(BUTTON_VOLUME_DOWN) != button_volume_down_state)
        {
            button_volume_down_state = !button_volume_down_state;
            if (button_volume_down_state)
            {
                printf("Clicked VOLUME DOWN\n");
                set_mode(IDLE);
            }
            else
                printf("Released VOLUME DOWN\n");
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
    xTaskCreate(gpio_task_handler, "gpio_task", 4 * 1024, NULL, 10, &s_gpio_task_handle);
}
void gpio_deinit()
{
    if (s_gpio_task_handle)
    {
        vTaskDelete(s_gpio_task_handle);
        s_gpio_task_handle = NULL;
    }
}
