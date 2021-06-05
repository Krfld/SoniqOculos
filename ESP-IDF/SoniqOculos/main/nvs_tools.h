#include "main.h"

#define NVS_TAG "NVS"

void nvs_init();
////void nvs_deinit();

void nvs_write(int32_t value);
int32_t nvs_read();