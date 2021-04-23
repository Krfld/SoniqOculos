#include "tools.h"

void delay(int millis)
{
    vTaskDelay(millis / portTICK_RATE_MS);
}

void handleMsgs(char *msg, size_t len)
{
    printf("%s\n", msg);

    // Response
    sprintf(msg, "Message received");
}