#include "main.h"

#define DEFAULT_READ_VALUE 1

void nvs_init();
////void nvs_deinit();

void nvs_write(int32_t value, char *namespace);
int32_t nvs_read(char *namespace);