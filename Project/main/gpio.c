#include "gpio.h"

#include "i2s.h"
#include "sd.h"

static xTaskHandle s_gpio_task_handle = NULL;

static void gpio_task_handler(void *arg);

void wait_for_sd_card()
{
    gpio_pad_select_gpio(SD_DET_PIN);
    gpio_set_direction(SD_DET_PIN, GPIO_MODE_INPUT);
    //gpio_set_pull_mode(SD_DET_PIN, GPIO_PULLDOWN_ONLY);

    do // Make sure the card is inserted
    {
        while (!gpio_get_level(SD_DET_PIN))
            ;
        delay(SD_CARD_DET_DELAY);
    } while (!gpio_get_level(SD_DET_PIN));
}

void speakers_pin_reset()
{
    gpio_pad_select_gpio(SPEAKERS_WS_PIN);
    gpio_set_direction(SPEAKERS_WS_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(SPEAKERS_WS_PIN, LOW);

    gpio_pad_select_gpio(SPEAKERS_BCK_PIN);
    gpio_set_direction(SPEAKERS_BCK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(SPEAKERS_BCK_PIN, LOW);

    gpio_pad_select_gpio(SPEAKERS_DATA_PIN);
    gpio_set_direction(SPEAKERS_DATA_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(SPEAKERS_DATA_PIN, LOW);
}

void microphones_pin_reset()
{
    gpio_pad_select_gpio(MICROPHONES_WS_PIN);
    gpio_set_direction(MICROPHONES_WS_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(MICROPHONES_WS_PIN, LOW);

    gpio_pad_select_gpio(MICROPHONES_BCK_PIN);
    gpio_set_direction(MICROPHONES_BCK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(MICROPHONES_BCK_PIN, LOW);

    gpio_pad_select_gpio(MICROPHONES_DATA_PIN);
    gpio_set_direction(MICROPHONES_DATA_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(MICROPHONES_DATA_PIN, LOW);
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
    //gpio_set_pull_mode(SD_DET_PIN, GPIO_PULLDOWN_ONLY);

    for (;;)
    {
        delay(DEBOUNCE);

        if (gpio_get_level(BUTTON_START) != button_start_state)
        {
            button_start_state = !button_start_state;
            if (button_start_state)
                printf("Clicked START\n");
            else
                printf("Released START\n");
        }

        if (gpio_get_level(BUTTON_VOLUME_UP) != button_volume_up_state)
        {
            button_volume_up_state = !button_volume_up_state;
            if (button_volume_up_state)
                printf("Clicked VOLUME UP\n");
            else
                printf("Released VOLUME UP\n");
        }

        if (gpio_get_level(BUTTON_VOLUME_DOWN) != button_volume_down_state)
        {
            button_volume_down_state = !button_volume_down_state;
            if (button_volume_down_state)
                printf("Clicked VOLUME DOWN\n");
            else
                printf("Released VOLUME DOWN\n");
        }

        if (gpio_get_level(SD_DET_PIN) != sd_det_state)
        {
            sd_det_state = !sd_det_state;
            if (sd_det_state)
                printf("Inserting card...\n");
            else

                printf("Removing card...\n");
        }
    }
}

void gpio_task_start_up(void)
{
    xTaskCreate(gpio_task_handler, "gpio_task_handler", 1024, NULL, 10, &s_gpio_task_handle);
}

void gpio_task_shut_down(void)
{
    if (s_gpio_task_handle)
    {
        vTaskDelete(s_gpio_task_handle);
        s_gpio_task_handle = NULL;
    }
}
