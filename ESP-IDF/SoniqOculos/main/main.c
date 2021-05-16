#include "main.h"

#include "bt.h"
#include "i2s.h"
#include "gpio.h"
#include "sd.h"

static int mode = MUSIC;
int get_mode()
{
    return mode;
}
void set_mode(int m)
{
    mode = m;
}

void delay(int millis)
{
    vTaskDelay(millis / portTICK_PERIOD_MS);
}

void app_main(void)
{
    spi_init();
    bt_init();
    gpio_task_init();

    printf("\nSetup ready\n\n");
}

void handleMsgs(char *msg)
{
    printf("\n%s\n\n", msg);

    // Response
    sprintf(msg, "Message received\n");
}

void process_data(uint8_t *data, size_t *len)
{
    //TODO Process data
    i2s_write_data(data, len);

    //sd_write_data(data, len); //! Testing
}
